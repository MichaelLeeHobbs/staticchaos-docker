# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Working Agreement

You're a partner, not an order-taker. **I own the vision and the final call; you own the option
space, the risks, and the better path** — then you execute once we're aligned. I have deep
experience, but the field is vast and you're trained across all of it (plus search), so part of your
job is telling me what I don't know or haven't considered.

- **Challenge the goal, not just the code.** If the requested approach — or the goal itself — looks
  wrong, say so and explain *before* doing it. "Is this even the right thing to build?" is fair game.
  Push back plainly; don't flatter. I'll overrule if I disagree — speaking up costs nothing.
- **Show the option space** (the fix for one-track thinking). At a design/decision point, surface
  2-3 viable approaches with tradeoffs + your pick instead of running with the first idea; when stuck
  or the obvious path is ugly, question the framing (different decomposition? different tool? a way to
  sidestep it?). Volunteer prior art or standard solutions I may not know — especially in domains I'm
  less deep in — and say what they're based on so I can verify. Diverge hard here; don't bikeshed
  trivial mechanical work.
- **Pivot on evidence.** If an approach is fighting you after a couple of honest attempts, stop —
  don't grind. Say "this is fighting us, here's why," and propose the alternative. A sunk-cost grind
  is worse than a pivot.
- **Surface, then act.** Spot a risk I may have missed (a security issue, a hard-to-reverse step, a
  cheaper path)? Raise it in a line and proceed with a recommendation — don't silently comply, and
  don't block on it either.
- **Reuse, don't regenerate.** Check for an existing helper/command/pattern before writing a new one.
  When something recurs (~3rd time), capture it once: a command → this file, a workflow → a skill,
  repeated code → a shared module. Don't abstract a one-off; don't keep rewriting a non-trivial thing.
- **Verification is non-negotiable.** Give yourself a check you can run (test / build / lint /
  screenshot); "looks done" isn't a signal. Don't grade your own work green — prefer an external,
  objective gate.

## Static Chaos — Dockerized

A 1990s Merc/Diku-derived MUD server (Static Chaos / Chaosium, C, by Nicholas "Alathon" Lennig),
packaged to build/run in Docker on a modern host and **actively developed** — ongoing
gameplay/balance work and new systems (GMCP, class trials) in `src/`, plus modern tooling around it:
Node scripts that parse the game's `.are` data into navigable references, and a React companion
website that publishes them. **The server in `src/`/`area/`/`doc/` is inherited C we didn't originally
write** — change it deliberately (understand the surrounding code, keep diffs focused, gate every
change through the Docker build — it only compiles 32-bit), but it is no longer frozen. New tooling
and site work lives in `tools/`, `web/`, and `docs/`.

### Three subsystems (the big picture)

1. **The MUD server** — `src/` (C), game data in `area/` (`.are` files + `area.lst`), runtime state
   in `player/` `notes/` `finger/` `log/`, original docs in `doc/`. Builds and runs only via Docker
   (see below); not meant to compile natively on Windows. `src/core/gmcp.c` adds GMCP (telnet opt 201)
   on top of the legacy line parser.
2. **World-data tooling** — `tools/*.mjs` (Node, ESM). Parse the `.are` files into `world-maps/`
   (JSON + Markdown + PDF + standalone HTML). The shared parser is `tools/lib/area.mjs` (Reader +
   section parsers + flag decoders) and it **must stay in sync with `src/core/db.c`**, which is the
   authoritative format; `build-classes.mjs` reads `class_table`/`skill_table` from `src/core/const.c`.
3. **Web companion site** — `web/` (Vite + React 19 + MUI + react-router + TypeScript). A static SPA
   that fetches the generated content. It does **not** read the repo directly: `web/scripts/sync-content.mjs`
   copies `world-maps/*.json` → `public/data/`, selected `docs/*.md` + `world-maps/*.md` → `public/docs/`,
   and `client/*.xml` (Mudlet GMCP packages) → `public/gmcp/`, each with an `index.json` manifest.

Data flow: `area/*.are` → (`tools/`) → `world-maps/` → (`web/scripts/sync-content.mjs`) → `web/public/` → SPA.

### Build & run

**MUD (Docker only):** `docker compose up --build` then `telnet localhost 4000`. Host port is
configurable via `MUD_PORT`; the container always listens on 4000 (code rejects ports ≤ 1024). Game
state lives in named volumes (`player`/`notes`/`finger`/`log`) and survives rebuilds; `docker compose
down -v` wipes it. Details, deploy scripts, and the documented source portability fixes are in
**`README.docker.md`** — read it before touching the Dockerfile or `src/`.

**World-data tooling (Node 18+; repo tested on Node 24):** run from repo root.
- `pnpm install` — needed only for the PDF steps (`md-to-pdf`, `@mermaid-js/mermaid-cli`).
- `pnpm run build:data` — runs every `build:*` JSON/MD/HTML generator (no browser needed).
- Individual generators: `build:world` `build:graph` `build:items` `build:bestiary` `build:spawns`
  `build:shops` `build:gearing` `build:classes` `build:browser` `build:d3map`.
- `pnpm run build:pdf` / `build:pdf:maps` — need a puppeteer Chrome (one-time
  `npx @puppeteer/browsers install chrome@<ver>`; the version must match what puppeteer-core pins).
- `pnpm run build` — data + PDFs (the full pipeline).

**Web site (in `web/`):** `pnpm install` then `pnpm dev` (runs sync + vite), `pnpm build`
(sync + `tsc -b` + vite build), `pnpm typecheck`, `pnpm serve` (production `server.mjs`). The web
build deploys via its own `web/Dockerfile` / `web/docker-compose.yml` (host :80).

### Conventions & gotchas

- **`tools/` ↔ `src/` coupling:** when game data format or flag bits change in `src/core/db.c` /
  `src/core/const.c`, update `tools/lib/area.mjs` to match — the parsers mirror the C, and drift produces
  silently wrong maps/items rather than errors. The `Fix_exits:` warnings at MUD boot are upstream
  area-data issues, not server bugs.
- **Adding a player-facing doc to the site:** a new `docs/*.md` is published only if its filename is
  added to the `USER_DOCS` set in `web/scripts/sync-content.mjs`. Internal planning docs (e.g.
  `GMCP-PLAN.md`, `*-BRAINSTORM.md`, `*-FEASIBILITY.md`) live in `docs/` but are intentionally not
  shipped — leave them out of `USER_DOCS`.
- **Generated artifacts are NOT committed:** `world-maps/` (JSON/MD/PDF/HTML) and `web/public/{data,docs,gmcp}`
  are build outputs and are gitignored (#65). Regenerate `world-maps/` with `pnpm run build:data` (JSON/MD/HTML;
  PDFs need `build:pdf*` + a puppeteer Chrome) — the web build/deploy and CI run `build:data` first, and
  `sync-content.mjs` fails loudly if it's missing. Never hand-edit a generated file; change the generator.
- **Source portability is deliberate:** the server is built 32-bit (`-m32`) on Debian Bullseye/gcc 10
  on purpose (it crashes 64-bit, and newer gcc rejects the legacy C). Passwords are stored in
  plaintext, inherited from the upstream codebase — do not expose this on the public internet assuming
  otherwise. The full rationale for every change is in `README.docker.md`.
- **Deploy scripts** (`deploy.sh`, `mud-redeploy.sh`, `web/deploy-web.sh`) are real, host-specific,
  and gitignored; only the `*.example.sh` templates are committed. `mud-redeploy.sh` deliberately
  aborts if players are online and verifies the build before restarting.
- **Magic indices in `src/`:** the legacy code indexes parallel arrays by bare ints; that idiom
  produced a recurring bug class (#44/#45/#48/#49). `pcdata->weapons[]` slots are now named
  `WP_HIT`..`WP_BLOW` (`merc.h`); use the name, not the number. `pnpm lint:c` (`tools/lint-c.mjs`)
  fails on any bare `weapons[<digit>]` — run it after touching class/combat code, and add rules there
  as more index families get named. CI also runs **cppcheck** on `src/` (buffer overflow, array OOB,
  null deref, format/scanf) and fails on new findings; known-intentional idioms and the 32-bit
  int↔pointer casts are muted in `.cppcheck-suppressions` (#62).
- **C formatting:** `.clang-format` captures the house style (4-space, Allman, `if ( x )` spacing).
  Use it on code you're editing — `pnpm run format:c -- <file>` — **never bulk-reformat** the legacy
  tree: its style is internally inconsistent, so a blanket run is pure churn that destroys `git blame`
  and risks the trigraph/`args(( ))` idioms. (Linting is separate: `lint:c` + cppcheck above.)

> **Maintaining this file:** keep it lean — prune rules the model already follows; when a section is
> only sometimes needed or keeps growing, move it to a doc or a skill and link it. Test for any line:
> would removing it cause a worse outcome?

### Workflow
**Default to background agents; you orchestrate, don't implement.** For non-trivial work, dispatch a
background agent in an isolated worktree and keep talking; merge → gate → prune on done. The three
subsystems parallelize cleanly, but each has a different gate (web typechecks locally; `src/` only
compiles in Docker) — full procedure and per-area gates: `.claude/skills/orchestrate/`.

### Active work
The MUD is under active development. In-flight design/planning lives in `docs/`:
`GMCP-PLAN.md` (GMCP telnet side-channel — Phases 0–3 implemented & deployed; `src/core/gmcp.c`,
`client/*.xml` Mudlet packages), `COMPUTER-PLAYERS-FEASIBILITY.md`, and
`SORCERER-AND-ADMIN-BRAINSTORM.md`. Read the relevant plan before touching its code.
