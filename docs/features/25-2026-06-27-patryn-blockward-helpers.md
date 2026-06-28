---
issue: 25
type: tech-debt
status: spec-review
opened: 2026-06-27
reporter: code-review (#3/#4/#8)
---

# Shared Patryn block/ward cascade helpers (#25)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/25

## Summary
The "find the highest-priority active Patryn block / ward" cascade is copy-pasted across
abilities. Extract two helpers so the priority lists live in one place — no behavior change.
Flagged by the #4 and #8 reviews; CLAUDE.md "reuse, don't regenerate (~3rd time, capture it)".

## Current duplication (verified)
- **`do_ryuken`** (`src/saiyan.c`, #4): block cascade (Air>Fire>Negative, ~1118-1126) + ward
  cascade (Spirit>Earth>Flame>Wind>Water>Negative, ~1138).
- **`chant_flow_break`** (`src/sorcerer.c`, #8): same two cascades (~1266 / ~1270).
- **`do_runeweave` case 513** (`src/patryn.c`, #3) and others touch the block bits ad hoc.

Each enumerates `NEW_AIR_BLOCK/NEW_FIRE_BLOCK/NEW_NEGATIVE_BLOCK` and the six `gsn_*_ward` by
hand; adding/renaming a block bit or ward gsn means editing every copy (silent-drift risk, no
compile error to catch a miss).

## Proposed design
Add two helpers in `src/patryn.c` (Patryn domain), prototyped in `merc.h` alongside `get_runes`:

```c
/* Highest-priority active rune block, or 0; sets *name to "air"/"fire"/"negative". */
int  patryn_active_block( CHAR_DATA *ch, char **name );
/* Highest-priority active ward's gsn, or 0 (Spirit>Earth>Flame>Wind>Water>Negative). */
int  patryn_active_ward ( CHAR_DATA *ch );
```

- `patryn_active_block`: returns the block bit (or 0) by priority Air>Fire>Negative, sets `*name`.
- `patryn_active_ward`: returns the ward gsn (or 0) by the documented ward priority.

Refactor `do_ryuken` (#4) and `chant_flow_break` (#8) to call them (callers still do the actual
`REMOVE_BIT` / `affect_strip` / `dec_duration` + messaging). Adopt in patryn.c case 513 where the
same selection applies.

## Acceptance criteria
- [ ] Helpers exist in patryn.c, prototyped in merc.h; both compile (32-bit Docker).
- [ ] `do_ryuken` and `chant_flow_break` use the helpers; their **observable behavior is unchanged**
      (same block/ward chosen, same priority) — verify with a targeted dev test of each ability.
- [ ] No remaining hand-rolled block/ward priority cascade in those two functions.

## Risk & balance notes
- Pure refactor, no behavior/number change. Main risk is an off-by-priority transcription — mirror
  the existing order exactly and dev-test ryuken + flow break vs a Patryn with blocks/wards.

## Out of scope
- Changing block/ward priorities or effects; the broader "one ward list everywhere" (db.c gsn list
  + const.c skill table) consolidation.
