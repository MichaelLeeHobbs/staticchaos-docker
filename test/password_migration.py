#!/usr/bin/env python3
"""
Password hashing + transparent-migration integration test for Static Chaos.

Companion to mud_smoke.py. Drives real telnet sessions against a running server
and inspects the on-disk pfile (via `docker exec`) to prove:

  1. New account: the created character's stored Password is a yescrypt HASH
     ("$y$..."), NOT plaintext; the correct password logs in; a wrong password
     is rejected.
  2. Legacy migration: a pre-existing pfile whose Password is stored in PLAINTEXT
     logs in successfully with that plaintext AND is transparently re-hashed on
     that login (the on-disk Password becomes "$y$..."); a second login (now
     against the hash) still works.

Needs `docker exec` to read/edit pfiles inside the `player` volume, so it only
runs meaningfully where docker is reachable (CI / Linux host). Exit 0 = pass.

Usage: python3 test/password_migration.py [--host H] [--port N] [--container NAME] [--class C]
Stdlib only.
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
GAME_PROMPT = re.compile(r"\d+H\s+\d+M\s+\d+V")


def clean(b: bytes) -> str:
    return ANSI.sub("", b.decode("latin-1", "replace"))


class Session:
    def __init__(self, host, port):
        self.s = socket.create_connection((host, port), timeout=10)
        self.buf = ""

    def read_until(self, pattern, timeout=15):
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
        self.buf = ""

    def close(self):
        try:
            self.s.close()
        except OSError:
            pass


def advance_to_game(sess, steps=25):
    for _ in range(steps):
        if GAME_PROMPT.search(sess.buf):
            return
        try:
            sess.s.settimeout(3)
            chunk = sess.s.recv(4096)
        except socket.timeout:
            sess.s.sendall(b"\n")
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


def dexec(container, *cmd, check=False):
    return subprocess.run(["docker", "exec", container, *cmd],
                          capture_output=True, text=True, timeout=15, check=check)


def rm_pfile(container, name):
    for p in (pfile_path(name), f"/mud/player/temp/{name}"):
        try:
            dexec(container, "rm", "-f", p)
        except (OSError, subprocess.SubprocessError):
            return


def docker_ok(container):
    try:
        return dexec(container, "true").returncode == 0
    except (OSError, subprocess.SubprocessError):
        return False


def password_line(container, name):
    """Return the raw stored Password value (between 'Password' and the '~'), or None."""
    r = dexec(container, "sh", "-c", f"grep -i '^Password' {pfile_path(name)} || true")
    m = re.search(r"^Password\s+(.*?)~", r.stdout, re.I | re.M)
    return m.group(1) if m else None


def make_char(host, port, name, pw, cls):
    s = Session(host, port)
    s.read_until(r"name", 15)
    s.send(name); s.read_until(r"\(Y/N\)"); s.send("y")
    s.read_until(r"password"); s.send(pw); s.read_until(r"retype"); s.send(pw)
    s.read_until(r"sex"); s.send("M"); s.read_until(r"destin|class"); s.send(cls)
    advance_to_game(s)
    return s


def login(host, port, name, pw):
    """Log an existing character in and reach the game prompt. Returns the session."""
    s = Session(host, port)
    s.read_until(r"name", 15)
    s.send(name)
    s.read_until(r"password", 10)   # existing char -> password, not "(Y/N)"
    s.send(pw)
    advance_to_game(s)
    return s


def login_expect_reject(host, port, name, pw):
    """Attempt a login with a bad password; assert it's rejected."""
    s = Session(host, port)
    s.read_until(r"name", 15)
    s.send(name)
    s.read_until(r"password", 10)
    s.send(pw)
    out = s.read_until(r"wrong password", 10)
    s.close()
    return out


def rand_name(prefix):
    return prefix + "".join(random.choice(string.ascii_lowercase) for _ in range(5))


def scenario_new_account(host, port, container, cls):
    name = rand_name("Hash")
    pw = "hashpw123"
    rm_pfile(container, name)
    print(f"\n=== Scenario 1: new account is stored as a hash ({name}) ===")
    s = make_char(host, port, name, pw, cls)
    s.send("save"); s.read_until(r"\bOk\b", 10)
    s.send("quit"); time.sleep(1.0); s.close()

    stored = password_line(container, name)
    print(f"    pfile Password after create: {stored!r}")
    assert stored is not None, "no Password line found in new pfile"
    assert stored.startswith("$y$"), f"new-account password is NOT a yescrypt hash: {stored!r}"
    assert pw not in stored, "plaintext password leaked into the stored hash"
    print("    [ok] stored value is a $y$ hash, not plaintext")

    s2 = login(host, port, name, pw)
    s2.send("score"); out = s2.read_until(GAME_PROMPT.pattern, 10)
    assert name.lower() in out.lower(), "correct password did not log the character in"
    s2.send("quit"); time.sleep(0.5); s2.close()
    print("    [ok] correct password logs in")

    login_expect_reject(host, port, name, "wrongpw999")
    print("    [ok] wrong password is rejected")
    rm_pfile(container, name)


def scenario_legacy_migration(host, port, container, cls):
    name = rand_name("Legacy")
    seed_pw = "seedpw123"
    legacy_pw = "secret123"
    rm_pfile(container, name)
    print(f"\n=== Scenario 2: legacy plaintext -> hash on login ({name}) ===")
    # Create a normal (hashed) character to get a fully valid pfile, then quit.
    s = make_char(host, port, name, seed_pw, cls)
    s.send("save"); s.read_until(r"\bOk\b", 10)
    s.send("quit"); time.sleep(1.0); s.close()

    # Downgrade the Password field to a PLAINTEXT value -> simulate a legacy
    # pre-hashing account. The rest of the pfile stays valid.
    dexec(container, "sh", "-c",
          f"sed -i 's/^Password.*/Password     {legacy_pw}~/' {pfile_path(name)}", check=True)
    before = password_line(container, name)
    print(f"    pfile Password before login (legacy): {before!r}")
    assert before == legacy_pw, f"failed to inject legacy plaintext (got {before!r})"

    # Log in with the plaintext password -> should succeed AND re-hash on the way in.
    s2 = login(host, port, name, legacy_pw)
    s2.send("score"); out = s2.read_until(GAME_PROMPT.pattern, 10)
    assert name.lower() in out.lower(), "legacy plaintext login failed"
    s2.send("quit"); time.sleep(1.0); s2.close()
    print("    [ok] legacy plaintext password logs in")

    after = password_line(container, name)
    print(f"    pfile Password after  login (migrated): {after!r}")
    assert after is not None and after.startswith("$y$"), \
        f"password was NOT migrated to a hash on login: {after!r}"
    assert after != before, "password unchanged after login (no migration happened)"
    print("    [ok] stored password was transparently upgraded to a $y$ hash")

    # Log in again -- now exercising the hash verify path.
    s3 = login(host, port, name, legacy_pw)
    s3.send("score"); out = s3.read_until(GAME_PROMPT.pattern, 10)
    assert name.lower() in out.lower(), "post-migration hash login failed"
    s3.send("quit"); time.sleep(0.5); s3.close()
    print("    [ok] second login (hash path) still works")

    # And the old plaintext is no longer accepted as-a-hash oddity: wrong pw rejected.
    login_expect_reject(host, port, name, "nope99999")
    print("    [ok] wrong password still rejected post-migration")
    rm_pfile(container, name)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=4000)
    ap.add_argument("--container", default="staticchaos")
    ap.add_argument("--charclass", default="Fist")
    args = ap.parse_args()

    if not docker_ok(args.container):
        print("SKIP: docker exec not reachable here; this test needs pfile access.",
              file=sys.stderr)
        return 0

    try:
        scenario_new_account(args.host, args.port, args.container, args.charclass)
        scenario_legacy_migration(args.host, args.port, args.container, args.charclass)
        print("\nPASSWORD MIGRATION TEST PASSED")
        return 0
    except (AssertionError, TimeoutError, OSError, subprocess.SubprocessError) as e:
        print(f"\nPASSWORD MIGRATION TEST FAILED: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
