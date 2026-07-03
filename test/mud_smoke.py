#!/usr/bin/env python3
"""
Integration smoke test for the Static Chaos MUD (issue #69).

Drives a real telnet session against a running server and asserts end-to-end
behaviour that unit-level checks can't reach: the nanny/login flow (comm.c),
character creation, command dispatch, and -- most importantly -- the
save -> quit -> log back in roundtrip, which exercises the whole save/load path
(save.c / db.c) that the #61 defensive-loader work protects.

Exit 0 = all scenarios passed; non-zero = a regression (with a clear message).

Usage:  python3 test/mud_smoke.py [--host H] [--port N] [--container NAME] [--class C]
Defaults target the local `docker compose` service (host 127.0.0.1:4000,
container `staticchaos`). No third-party deps -- stdlib only.
"""
import argparse
import random
import re
import socket
import string
import subprocess
import sys
import time

ANSI = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")
# The in-game prompt always carries the H/M/V vitals, e.g. "[2000H 1000M 1000V]".
GAME_PROMPT = re.compile(r"\d+H\s+\d+M\s+\d+V")


def clean(b: bytes) -> str:
    return ANSI.sub("", b.decode("latin-1", "replace"))


class Session:
    def __init__(self, host, port):
        self.s = socket.create_connection((host, port), timeout=10)
        self.buf = ""

    def read_until(self, pattern, timeout=15):
        """Accumulate output until `pattern` (regex) appears; return the text.
        Raises TimeoutError with the tail of what was seen on failure."""
        rx = re.compile(pattern, re.I)
        deadline = time.time() + timeout
        while time.time() < deadline:
            if rx.search(self.buf):
                return self.buf
            self.s.settimeout(max(0.1, deadline - time.time()))
            try:
                chunk = self.s.recv(4096)
            except socket.timeout:
                break
            if not chunk:
                break
            self.buf += clean(chunk)
        raise TimeoutError(f"expected /{pattern}/; last saw:\n...{self.buf[-300:]!r}")

    def send(self, line):
        self.s.sendall(line.encode() + b"\n")
        self.buf = ""  # reset so the next read_until only sees fresh output

    def close(self):
        try:
            self.s.close()
        except OSError:
            pass


def advance_to_game(sess, steps=25):
    """Answer the variable post-class-selection / motd prompts until the in-game
    prompt (vitals) appears. Robust to the stat-roll and motd-pager screens."""
    for _ in range(steps):
        if GAME_PROMPT.search(sess.buf):
            return
        try:
            sess.s.settimeout(3)
            chunk = sess.s.recv(4096)
        except socket.timeout:
            sess.s.sendall(b"\n")  # nudge a pager
            continue
        if not chunk:
            break
        sess.buf += clean(chunk)
        low = sess.buf.lower()
        if GAME_PROMPT.search(sess.buf):
            return
        if any(k in low for k in ("acceptable", "reroll", "keep these", "(y/n)")):
            sess.s.sendall(b"y\n"); sess.buf = ""
        elif any(k in low for k in ("return to continue", "press return", "[ press", "hit return")):
            sess.s.sendall(b"\n"); sess.buf = ""
    raise TimeoutError(f"never reached the in-game prompt; last saw:\n...{sess.buf[-300:]!r}")


def pfile_path(name):
    return f"/mud/player/{name[0].lower()}/{name}"


def rm_pfile(container, name):
    """Best-effort cleanup of the test character's save file so the volume
    doesn't accumulate. Never fails the test: `docker` may be unreachable from
    this interpreter (e.g. Windows), and a unique name already guarantees a
    fresh CREATE without needing pre-cleanup."""
    for p in (pfile_path(name), f"/mud/player/temp/{name}"):
        try:
            subprocess.run(["docker", "exec", container, "rm", "-f", p],
                           capture_output=True, timeout=10)
        except (OSError, subprocess.SubprocessError):
            return  # docker not callable here -- skip cleanup silently


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=4000)
    ap.add_argument("--container", default="staticchaos")
    ap.add_argument("--charclass", default="Fist")
    ap.add_argument("--name", default=None,
                    help="character name (default: a unique Smoke<ts> name per run)")
    args = ap.parse_args()
    # A unique name per run means the first connect always hits CREATE (not LOGIN)
    # without depending on pre-cleanup. Names must be alphabetic (digits are
    # rejected as "Illegal name"), so use a random lowercase-letter suffix.
    if not args.name:
        args.name = "Smoke" + "".join(random.choice(string.ascii_lowercase) for _ in range(5))
    pw = "smokepw1"

    # Best-effort: clear any stale save for this name so the first connect creates.
    rm_pfile(args.container, args.name)

    try:
        # --- 1. connect + banner ---
        s = Session(args.host, args.port)
        s.read_until(r"name", 15)
        print("[1/6] connected; name prompt reached")

        # --- 2. create a character through the nanny ---
        s.send(args.name)
        s.read_until(r"\(Y/N\)")
        s.send("y")
        s.read_until(r"password")
        s.send(pw)
        s.read_until(r"retype")
        s.send(pw)
        s.read_until(r"sex")
        s.send("M")
        s.read_until(r"destin|class")
        s.send(args.charclass)
        advance_to_game(s)
        print(f"[2/6] created {args.name} the {args.charclass}; in game")

        # --- 3. a command returns a sane response ---
        s.send("score")
        out = s.read_until(GAME_PROMPT.pattern, 10)
        assert args.name.lower() in out.lower(), "score did not show the character name"
        print("[3/6] 'score' responds and shows the character")

        # --- 4. save confirms ---
        s.send("save")
        s.read_until(r"\bOk\b", 10)
        print("[4/6] 'save' confirmed")

        # --- 5. quit (persists the character) ---
        s.send("quit")
        time.sleep(1.0)
        s.close()
        print("[5/6] quit (character saved)")

        # --- 6. log back in: exercises the full save->load roundtrip ---
        s2 = Session(args.host, args.port)
        s2.read_until(r"name", 15)
        s2.send(args.name)
        s2.read_until(r"password", 10)   # existing char -> password, not "(Y/N)"
        s2.send(pw)
        advance_to_game(s2)
        s2.send("score")
        out = s2.read_until(GAME_PROMPT.pattern, 10)
        assert args.name.lower() in out.lower(), "reloaded character lost its identity"
        s2.send("quit"); time.sleep(0.5); s2.close()
        print("[6/6] logged back in and character reloaded intact (save/load roundtrip)")

        print("\nSMOKE TEST PASSED")
        return 0
    except (AssertionError, TimeoutError, OSError) as e:
        print(f"\nSMOKE TEST FAILED: {e}", file=sys.stderr)
        return 1
    finally:
        rm_pfile(args.container, args.name)


if __name__ == "__main__":
    sys.exit(main())
