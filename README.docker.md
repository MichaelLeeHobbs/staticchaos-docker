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

Create a new character at the "By what name do you wish to be known?" prompt. (Earlier builds
shipped a default `Superuser`/`superuser` account; it's been removed from the repo so no
credentials are published — create your own.) To make a character an immortal, stop the server
and set its `Level` (17–20; `MAX_LEVEL` is 20) plus `Trust`/`Wizbit` in its player file.

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

## Deploying to a remote host

The repo ships **example** deploy scripts that push this project to a remote Docker host over
SSH. They're templates — copy each to the real name (the real ones are gitignored so your
host/paths stay out of the repo) and set your host:

```bash
cp deploy.example.sh            deploy.sh
cp mud-redeploy.example.sh      mud-redeploy.sh
cp web/deploy-web.example.sh    web/deploy-web.sh
# then either edit REMOTE in each, or pass it at runtime:
REMOTE=user@host ./deploy.sh
```

| Script | What it does |
|--------|--------------|
| `deploy.sh` | Full stack: package the project, copy to `REMOTE_DIR` on the host, `docker compose up -d --build` (MUD + web). |
| `mud-redeploy.sh` | MUD only, **safe**: aborts if players are online, builds (verify) *before* restarting so a bad compile never kills the running game. Use for source/area changes. |
| `web/deploy-web.sh` | Web companion site only (rebuilds the `web` compose service on :80). |

All three honor `REMOTE` (SSH alias or `user@host`) and `REMOTE_DIR` env vars and derive their
own source path, so they work from any checkout. They need `ssh`/`scp`/`tar` and key-based SSH
to the host (run from WSL or Git Bash on Windows). Named volumes mean game data survives deploys.

## Data persistence

Mutable game data lives in named volumes so accounts and progress survive
`docker compose up --build` and restarts:

| Volume   | Mount          | Contents                                   |
|----------|----------------|--------------------------------------------|
| `area`   | `/mud/area`    | World data (`.are`) + in-game OLC builds    |
| `player` | `/mud/player`  | Player files (`/<initial>/<Name>`) + `temp`|
| `notes`  | `/mud/notes`   | Note board data                            |
| `finger` | `/mud/finger`  | Player "finger" info                        |
| `log`    | `/mud/log`     | Runtime log directory                      |

`area` is a volume because the server auto-dumps the **whole world** to `/mud/area` every 30 min
(`PULSE_DB_DUMP` → `do_asave`), and in-game OLC builds write there too — without a volume those
rewrites live only in the container's writable layer and are lost on the next rebuild/recreate.

On first run Docker seeds these volumes from the image. Runtime contents (player files, notes,
finger info, logs) are gitignored — a fresh clone ships only empty directories and no
credentials, so the volumes start clean. **The committed `area/` is the seed for the `area`
volume**, so on a fresh host the current world comes along; an *existing* non-empty volume is
used as-is (never re-seeded).

To wipe all game state and start fresh:

```bash
docker compose down -v
```

> ⚠️ Because an empty `area` volume seeds from the image, after `down -v` you must
> `docker compose up --build` (so the image carries the committed `area/`) — or re-seed from a
> backup (below) — or you'll boot an out-of-date world.

### Backups & restore

`backup-volumes.example.sh` (copy to a gitignored `backup-volumes.sh`, schedule via cron) snapshots
every volume to rotating tarballs **outside** the build context — covering host disk loss and an
accidental `docker compose down -v` (the volumes alone only survive container recreate). To restore
one volume from a tarball:

```bash
# inspect a backup
tar tzf /opt/backups/staticchaos/staticchaos_area-<stamp>.tgz | head

# restore into the live volume (stop the MUD first to avoid the 30-min auto-dump
# overwriting your restore):
docker compose stop mud
docker run --rm \
  -v staticchaos_area:/dst \
  -v /opt/backups/staticchaos:/src:ro \
  alpine sh -c 'rm -rf /dst/* && tar xzf /src/staticchaos_area-<stamp>.tgz -C /dst'
docker compose start mud
```

Swap the volume name / tarball for `player`, `notes`, etc. The tarballs are flat (`tar … -C /src .`),
so they extract directly into the volume root.

## Resilience / self-heal

`restart: unless-stopped` only heals **transient** crashes (crash → restart → boots clean, e.g. a
segfault mid-play; players just reconnect). It **cannot** heal a **deterministic boot failure** (bad
data → `exit(1)` on load → restart → same exit) — that just loops forever, which is what once took
the server down. Three layers cover both cases:

1. **Boot tolerance (code).** A dangling mob/obj vnum in a reset/special/shop/mobprog is logged and
   skipped instead of `exit(1)` (`get_mob_index`/`get_obj_index`, `src/db.c`). One bad record
   degrades that record, not the whole game. (Room vnums stay fatal — they're structural.)
2. **Healthcheck (compose).** A `bash /dev/tcp` connect to 4000 marks the container `healthy` /
   `unhealthy` in `docker ps` (and feeds the watchdog). It opens+closes a socket every 30s, which the
   MUD logs as a short-lived connection — harmless, tune `interval` if the noise bothers you.
3. **Self-heal watchdog (host cron).** `watchdog.sh` (`/etc/cron.d/staticchaos-watchdog`, every 2 min)
   acts only on **sustained** trouble (default 5 min — long enough to ride out a normal restart). It
   snapshots the failed `area` volume to `area-FAILED-<stamp>.tgz` for forensics, then restores the
   newest backup that actually boots (walking back if needed) and restarts, with a 1 h cooldown so a
   still-broken box isn't healed in a loop (it escalates to the log instead). Log + state live in
   `/opt/backups/staticchaos/` (`watchdog.log`, `watchdog.state`). Alerts are host-log only for now —
   swap the `alert()` body for a webhook/email to push them.

Tunables are at the top of `watchdog.sh` (`GRACE_SECS`, `COOLDOWN_SECS`, `VERIFY_WAIT`). To
disable temporarily: `rm /etc/cron.d/staticchaos-watchdog`. Fire-drill (verify it still works):
stop the container, truncate `school.are` in the volume, start it (it will boot-loop), then run
`watchdog.sh` with `first_trouble` backdated past the grace window in `watchdog.state`.

## World data tooling

Node scripts (in `tools/`) parse the `.are` files into navigable references under
`world-maps/`. Requires Node 18+. No build needed for the data; PDFs need one Chrome download.

```bash
pnpm install                 # installs md-to-pdf (for the PDF step)

pnpm run build:world         # world.json, WORLD-MAP.md, <area>.md, WORLD-ATLAS.md
pnpm run build:items         # items.json + ITEMS.md       (equipment catalogue)
pnpm run build:bestiary      # bestiary.json + BESTIARY.md
pnpm run build:spawns        # spawns.json + SPAWNS.md     (where to find mobs/items)
pnpm run build:shops         # shops.json + SHOPS.md       (economy)
pnpm run build:classes       # classes.json + CLASSES.md   (class/skill/spell ref)
pnpm run build:browser       # world-browser.html          (offline searchable browser)
pnpm run build:data          # all of the above

# One-time browser provision for PDF rendering (md-to-pdf uses puppeteer):
npx @puppeteer/browsers install chrome@149.0.7827.22 --path ~/.cache/puppeteer

pnpm run build:pdf           # PDFs of ATLAS, ITEMS, BESTIARY, SPAWNS, SHOPS, CLASSES
pnpm run build:pdf:maps      # WORLD-MAPS.pdf -- the Mermaid diagrams, rendered
pnpm run build               # build:data + build:pdf + build:pdf:maps
```

`build:pdf:maps` needs `@mermaid-js/mermaid-cli` (a dev dep) and the same puppeteer Chrome.
It pre-renders every Mermaid block to SVG (raising Mermaid's 500-edge cap) and prints them
to `WORLD-MAPS.pdf` in A3 landscape. Areas over ~30 rooms are auto-split into connected
sub-maps of ≤24 rooms (`partitionArea` in `tools/lib/area.mjs`) so each fits a page legibly;
exits to a sibling sub-map show as green `▸ Part X` stubs, exits leaving the area as grey
stubs. The same split is applied to the per-area `.md` files.

Outputs (all under `world-maps/`):

| File | What it is |
|------|------------|
| `world.json` | Canonical room graph + reachability/dangling-exit reports |
| `WORLD-MAP.md` | Stats, an area-connectivity Mermaid diagram, area index |
| `<area>.md` | Per-area Mermaid room map + room table (one per area) |
| `WORLD-ATLAS.md` / `.pdf` | Print-friendly atlas of every area/room/exit |
| `WORLD-MAPS.pdf` | The Mermaid diagrams (overview + every area) rendered to PDF |
| `ITEMS.md` / `.pdf`, `items.json` | Equipment catalogue: type, wear slot, stat affects, flags |
| `BESTIARY.md` / `.pdf`, `bestiary.json` | Every mob: level, behaviour flags, where it appears |
| `SPAWNS.md` / `.pdf`, `spawns.json` | "Where do I find X" — mob/item placement + hunting guide |
| `SHOPS.md` / `.pdf`, `shops.json` | Shops: keeper, location, trades, markups, stock |
| `CLASSES.md` / `.pdf`, `classes.json` | Classes (stat caps) + every skill/spell's learn level |
| `world-browser.html` | Self-contained offline browser — search & click-through (text/table) |
| `world-map.html` | Interactive D3 map — area graph → drill into rooms, pan/zoom/drag, portals, detail panel (D3 vendored inline, offline) |

The shared parser lives in `tools/lib/area.mjs` (Reader + section parsers + flag decoders,
matching `src/db.c`); `build-classes.mjs` reads the `class_table`/`skill_table` from
`src/const.c`. The PDF Chrome version (`chrome@…`) must match whatever `puppeteer-core` pins;
if `build:pdf` reports "Could not find Chrome (ver. X)", install that X.

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

4. **`src/Makefile` — 32-bit build (`-m32`)**
   - The server is compiled 32-bit. This code assumes `sizeof(int) == sizeof(pointer) == 4`; a 64-bit
     build links and boots and handles login fine, but **crashes during gameplay** (e.g. combat) when a
     truncated pointer is dereferenced, taking the whole process down. Building 32-bit is the standard,
     reliable fix for Diku/Merc-era code. The Dockerfile installs `gcc-multilib` + `libc6-dev-i386` for
     this. (Symptom if you ever revert to 64-bit: players get "remote host closed the connection" mid-play
     and the container's `RestartCount` climbs.)

## Verified working

- Boots and loads all area files → `Merc is ready to rock on port 4000.`
- Accepts telnet connections; renders the Static Chaos title screen and login.
- Create a character → MOTD → in-world at "Entrance to Mud School".
- New character creation (name → confirm → password → sex → class → MOTD → in-world).
- Player data persists across `docker compose restart` via the `player` volume.

The `Fix_exits: …` lines during boot are upstream area-data validation warnings (bad exit links
in the bundled `.are` files), not server errors.
