# Toolchain & the 64-bit Port Exit Plan

**Internal planning doc.** Not shipped to the website (deliberately absent from `USER_DOCS` in
`web/scripts/sync-content.mjs`). Audience: maintainers deciding whether/how to modernize the build.

## Current stance (what we build and why)

The `chaosium` server is compiled **32-bit (`-m32`), on Debian Bullseye with gcc 10**. This is
deliberate, not incidental. Two independent reasons:

1. **32-bit is a correctness requirement, not an optimization.** This 1990s Merc/Diku code assumes
   `sizeof(int) == sizeof(pointer) == 4`. Built 64-bit it *compiles* but crashes during gameplay:
   pointers that get stored in / cast through `int` are truncated, and the truncated pointer is later
   dereferenced, taking the whole process down. Compiling 32-bit is the standard, reliable fix for
   Diku/Merc-era code.
   - Cited: `src/Makefile` lines 7-10 (the `-m32` comment) and `README.docker.md` section 4,
     "`src/Makefile` — 32-bit build (`-m32`)" (lines ~253-258). Observed symptom of a 64-bit build:
     players get "remote host closed the connection" mid-play.

2. **gcc 10 is the newest compiler that still accepts the source.** gcc 14 promotes this code's
   implicit function declarations and implicit-`int` to hard errors; gcc 10 keeps them as warnings,
   so the source builds with only the Makefile tweaks we made (`-fcommon`, warning suppressions).
   - Cited: `Dockerfile` header comment (lines 3-7); `src/Makefile` lines 4-6 (`-fcommon`) and
     `README.docker.md` section on `-fcommon` (line ~225).

The Dockerfile installs `gcc-multilib` + `libc6-dev-i386` to provide the 32-bit toolchain and libs.
`crypt()` is macro'd to plaintext in `merc.h` (`#define crypt(s1,s2) (s1)`, merc.h:2785/2843) — faithful
to the upstream codebase; unrelated to word size but part of why this must not face the public internet.

## The concrete assumptions that block a 64-bit build

These are the things an actual port would have to fix. Evidence is from `src/`:

- **`int`-width type aliases.** `merc.h:56-57` defines, for the 32-bit build,
  `typedef int sh_int;` and `typedef int bool;`. The int/short choice is compile-time conditional, so
  the many places that assume a specific width are only correct under the current configuration.

- **Pointers/`time_t` narrowed through `int` in the save format.** Player save files are written with
  `%d` and explicit `(int)` casts of wider values. Example: `src/core/save.c:264`
  `fprintf( fp, "Note %d\n", (int)ch->last_note );` — `last_note` is a `time_t` (merc.h:1490), which is
  64-bit on a 64-bit host, so the cast truncates. Several `time_t` fields exist (`logon`, `save_time`,
  `last_note`, board `last_note[]`, `lastkill`), all persisted through the same `%d`/`fread_number`
  path. **Save-file field widths are format-visible:** changing the build word size can change what
  gets written, so existing player files are part of the compatibility surface.

- **Bit vectors packed into `int` fields.** Flags like `act`, `affected_by` are `int` and manipulated
  with `IS_SET`/bit macros (e.g. `src/core/save.c:275-278`). Any flag family that grows past 31 bits, or any
  code that conflates a flag word with a pointer, is width-sensitive.

- **The general pointer-in-`int` idiom.** Casts of the form `(int)<expr>` appear across the tree
  (`update.c`, `save.c`, `handler.c`, `fight.c`, `db.c`, `comm.c`, `board.c`, `act_wiz.c`,
  `act_move.c`, ...). Most are harmless int/int casts, but this is exactly the pattern that silently
  truncates a pointer on LP64 and is why the 64-bit build crashes rather than fails to compile.

(Note: the Merc memory pool in `db.c:2679-2729` stores free-list links *inside* freed blocks via
`*((void **) pMem)` — that one is width-agnostic and is **not** a blocker; listed here so a future
auditor doesn't flag it.)

## Scoping checklist for a future 64-bit port (NOT a commitment)

If someone decides to port, audit at least:

- [ ] **Type aliases** — resolve `sh_int`/`bool` widths (merc.h:56-61) and confirm every struct field
      and array index that relies on them.
- [ ] **All `(int)`/`(long)` casts of pointers or `time_t`** — grep `src/` for `(int)` / `(long)`
      applied to addresses, handles, or time values; fix the truncating ones.
- [ ] **Save/load format** — every `%d`/`%ld` in `save.c`, `board.c`, and `fread_number`/`fwrite_number`
      users. Decide on a format version bump and a migration path for existing `player/` files (the
      widths are format-visible, so this is the highest-risk area for data compatibility).
- [ ] **Bit-flag fields** — confirm no flag family overflows 31/32 bits and that no flag word is ever
      cast to/from a pointer.
- [ ] **`printf`/`scanf` format specifiers** vs. actual argument types once widths change.
- [ ] **Compiler modernization** — moving off gcc 10 means resolving the implicit-decl/implicit-int
      errors gcc 14 rejects; that is a separate, larger workstream from the word-size port and should
      be scoped independently.
- [ ] **Runtime soak test** — the 64-bit failure mode is a *runtime* crash (combat, etc.), not a build
      error, so any port needs real gameplay soak testing, not just a clean compile.

Recommendation: this is a large, low-reward change for an archival hobby server. The 32-bit build is
the pragmatic, supported configuration. Treat this doc as the map if the situation ever forces the
port (e.g. multilib disappears from future Debian).

## Maintenance stance

- **Base image is digest-pinned.** The Dockerfile pins `debian:bullseye-slim` by
  `@sha256:...` (tag kept in a trailing comment for humans) so rebuilds are reproducible even after the
  floating tag moves or Bullseye eventually EOLs. Re-pin intentionally: `docker pull debian:bullseye-slim`
  then `docker inspect --format='{{index .RepoDigests 0}}' debian:bullseye-slim`, and update the digest
  + comment.
- **Build-canary is planned, not here.** A CI "does it still build on newer gcc with `-m32`" canary
  belongs to the CI work in **issue #56** — it is intentionally *not* added by this doc or issue #63.
  Do not add a `.github/workflows/*` file as part of toolchain changes.
