---
issue: 54
type: tech-debt
status: in-progress
opened: 2026-07-02
reporter: maintainer
---

# Name remaining magic numbers: stances[], runeweave cases, extras[HOME] (#54)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/54

Follow-on to #52 (`weapons[]` → `WP_*`). Same bug-class prevention. All changes
behavior-preserving; **rename only, never renumber** (arrays are persisted
positionally in player files).

## 1. `do_runeweave` case labels (`src/patryn.c`)

`switch(spell)` where `spell` is an OR of `RUNE_*` bits. 12 cases were already in
`case RUNE_AIR + RUNE_LIFE:` form; the other 20 were raw numbers. Converted all 20
to the existing named style. Each decode verified against the in-file
`/* ELEM + CAT */` comment and arithmetic; `1028` = Balus Wall cross-checks #27.

| was | now | | was | now |
|----|-----|--|----|-----|
| 80 | `RUNE_ENERGY + RUNE_LIFE` | | 544 | `RUNE_NEGATIVE + RUNE_DESTRUCTION` |
| 96 | `RUNE_NEGATIVE + RUNE_LIFE` | | 1025 | `RUNE_AIR + RUNE_PROTECTION` |
| 129 | `RUNE_AIR + RUNE_DEATH` | | 1026 | `RUNE_EARTH + RUNE_PROTECTION` |
| 132 | `RUNE_FIRE + RUNE_DEATH` | | 1028 | `RUNE_FIRE + RUNE_PROTECTION` |
| 257 | `RUNE_AIR + RUNE_CREATION` | | 1032 | `RUNE_WATER + RUNE_PROTECTION` |
| 264 | `RUNE_WATER + RUNE_CREATION` | | 1056 | `RUNE_NEGATIVE + RUNE_PROTECTION` |
| 272 | `RUNE_ENERGY + RUNE_CREATION` | | 2049 | `RUNE_AIR + RUNE_TRANSFORMATION` |
| 513 | `RUNE_AIR + RUNE_DESTRUCTION` | | 2052 | `RUNE_FIRE + RUNE_TRANSFORMATION` |
| 516 | `RUNE_FIRE + RUNE_DESTRUCTION` | | 2064 | `RUNE_ENERGY + RUNE_TRANSFORMATION` |
| 528 | `RUNE_ENERGY + RUNE_DESTRUCTION` | | 2080 | `RUNE_NEGATIVE + RUNE_TRANSFORMATION` |

The now-redundant `/* ELEM + CAT */` comments are left in place (harmless, zero
diff risk).

## 2. `stances[]` → `STANCE_*` (`src/merc.h` + 46 sites)

`stance_table` order (const.c) is the source of truth:

| idx | macro | idx | macro |
|----:|-------|----:|-------|
| 0 | `STANCE_AUTOSLOT` * | 6 | `STANCE_HAWK` |
| 1 | `STANCE_LION` | 7 | `STANCE_EAGLE` |
| 2 | `STANCE_LYNX` | 8 | `STANCE_VULTURE` |
| 3 | `STANCE_SNAKE` | 9 | `STANCE_SPARROW` |
| 4 | `STANCE_BADGER` | 10 | `STANCE_STORK` |
| 5 | `STANCE_FERRET` | | |

**\* Slot 0 dual role:** `stances[0]` is *not* proficiency in stance 0 — it stores
the player's chosen autostance (written by `do_autostance` in act_move, read as
`autostance` in fight.c). Named `STANCE_AUTOSLOT` with a comment, distinct from the
stance-identity `0` = "no stance". Slots 1–10 are proficiency.

Linter rule added (`stances-magic-index`) — all slots nameable, so bare
`stances[<digit>]` is enforced to zero.

## 3. `extras[6]` → `extras[HOME]` (3 semantic sites)

`HOME` (6) already `#define`d. The 3 semantic sites (`act_move.c` ×2, `suit.c`)
do `get_room_index(extras[HOME])`. The 0–9 dumps (`save.c`, `act_wiz.c`) hit
unnamed slots 0–2, so they stay numeric and **no** extras linter rule is added.
`extras2[]` bare sites are all dumps — left as-is.

## Acceptance criteria
- [x] 20 runeweave cases converted to `RUNE_*` form (32 named total, 0 numeric).
- [x] `STANCE_*` defined; all 46 bare `stances[<digit>]` named; computed indices
      untouched; slot 0 named `STANCE_AUTOSLOT`.
- [x] 3 semantic `extras[6]` → `extras[HOME]`; dumps untouched.
- [x] No slot renumbered; save-file format byte-identical.
- [x] `pnpm lint:c` clean (2 rules: weapons + stances); negative-tested.
- [x] `docker compose build mud` compiles 32-bit; boots to "ready to rock".

## Risk
Very low — value-preserving `#define`/`case` substitution. Only real hazard is a
wrong name-to-number map, mitigated by the verified tables above and a
diff confirmed to touch only the target tokens.
