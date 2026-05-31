# Static Chaos MUD — Dockerized

This is the [Static Chaos](https://github.com/DikuMUDOmnibus/staticchaos) MUD (a 1990s
Merc/Diku-derived MUD server written in C, by Nicholas "Alathon" Lennig) packaged to build
and run as a Docker container on a modern host.

The upstream source is vendored here unchanged except for the small, documented portability
fixes described below. The original `README.md` (license/credits) is preserved alongside it.

## Quick start

```bash
# Build and run (foreground logs)
docker compose up --build

# …or detached
docker compose up --build -d
```

Then connect with any telnet/MUD client:

```bash
telnet localhost 4000
```

Log in as the bundled admin account: **`Superuser`** / password **`superuser`**, or create a
new character at the "By what name do you wish to be known?" prompt.

### Choosing the host port

The MUD process always listens on container port **4000** (the code rejects ports ≤ 1024).
The published *host* port is configurable — useful if 4000 is already in use:

```bash
MUD_PORT=4010 docker compose up -d      # connect on localhost:4010
```

### Plain Docker (no compose)

```bash
docker build -t staticchaos .
docker run -d -p 4000:4000 --name staticchaos staticchaos
telnet localhost 4000
```

## Data persistence

Mutable game data lives in named volumes so accounts and progress survive
`docker compose up --build` and restarts:

| Volume   | Mount          | Contents                                   |
|----------|----------------|--------------------------------------------|
| `player` | `/mud/player`  | Player files (`/<initial>/<Name>`) + `temp`|
| `notes`  | `/mud/notes`   | Note board data                            |
| `finger` | `/mud/finger`  | Player "finger" info                        |
| `log`    | `/mud/log`     | Runtime log directory                      |

On first run Docker seeds these volumes from the image (so the a–z player directories and the
bundled `Superuser` come along). To wipe all game state and start fresh:

```bash
docker compose down -v
```

## How it runs

The server binary is `chaosium`. It is launched from the `area/` directory
(`WORKDIR /mud/area`, `CMD ["/mud/src/chaosium", "4000"]`) because it loads `area.lst` and
reads/writes player and note files via paths relative to the working directory — exactly as the
original `startup` script did (`cd ../area`).

Server output (the boot sequence and `log_string` messages) goes to stdout/stderr, so use
`docker compose logs -f` (or `docker logs`) to watch it.

## Portability fixes applied to the source

This codebase targeted mid-1990s 32-bit Unix and gcc. Building it on Debian Bullseye (gcc 10)
required a handful of minimal, documented changes — each is commented in-place in the source:

1. **`src/Makefile`**
   - Added `-fcommon` to `C_FLAGS`. gcc 10+ defaults to `-fno-common`, which turns this code's
     tentative/common global definitions (e.g. `MOBtrigger`) into multiple-definition link
     errors. `-fcommon` restores the legacy behavior.
   - Added `LIBS = -lm` and appended it to the link line (math routines need libm).

2. **`src/comm.c`**
   - Joined a string literal that spanned three physical lines with raw (unescaped) newlines —
     invalid C — into adjacent concatenated literals (the "new character" lockout message).
   - Commented out the legacy `#if defined(linux)` hand-written prototypes for
     `read`/`write`/`gettimeofday`/`close`/`select`/`socket`. They conflict with modern glibc
     (`read`/`write` return `ssize_t` and take `void *`/`size_t`; `gettimeofday`'s second arg is
     `void *`); the correct declarations now come from the system headers.

3. **`src/merc.h`**
   - In the `#if defined(linux)` block, `#include <unistd.h>` is pulled in *before* the
     `#define crypt(s1,s2) (s1)` macro. Static Chaos historically stored passwords in **plaintext
     on Linux** (it never linked libcrypt — see the bare upstream Makefile and the bundled player
     files, whose passwords are plaintext). Modern glibc declares `crypt()` in `<unistd.h>`, which
     collided with that macro and broke compilation. Including `<unistd.h>` first (its include
     guard makes every later `#include <unistd.h>` a no-op) lets us keep the original plaintext
     behavior — so the bundled `Superuser` account still works — without the header collision.

> **Security note:** passwords are stored in plaintext, faithful to how this archived codebase
> ran on Linux. This is fine for a private/hobby server but do **not** expose it on the public
> internet with that assumption. Switching to real `crypt(3)` hashing would mean removing the
> `crypt` macro in `merc.h`, linking `-lcrypt`, and including `<crypt.h>` — at the cost of
> invalidating the existing plaintext passwords in the bundled/old player files.

No source changes were needed for 64-bit: a 64-bit build runs correctly (full login, world load,
character creation, and persistence all verified).

## Verified working

- Boots and loads all area files → `Merc is ready to rock on port 4000.`
- Accepts telnet connections; renders the Static Chaos title screen and login.
- Login as `Superuser` / `superuser` → MOTD → in-world at "Entrance to Mud School".
- New character creation (name → confirm → password → sex → class → MOTD → in-world).
- Player data persists across `docker compose restart` via the `player` volume.

The `Fix_exits: …` lines during boot are upstream area-data validation warnings (bad exit links
in the bundled `.are` files), not server errors.
