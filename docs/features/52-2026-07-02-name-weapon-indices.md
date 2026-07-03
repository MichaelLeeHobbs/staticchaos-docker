---
issue: 52
type: tech-debt
status: spec-review
opened: 2026-07-02
reporter: maintainer
---

# Name the magic weapon-proficiency indices (`weapons[]` → `WP_*`) + lint guard (#52)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/52

## Why

`pcdata->weapons[MAX_WEAPONS]` (13 weapon-proficiency slots) is indexed all over
the class/combat code with **bare integer literals** — `weapons[5]`, `weapons[11]`,
`weapons[0]` — with nothing saying what a slot *is*. This is the exact idiom that
produced this session's recurring bug class:

| Issue | Bug | Root cause |
|------|-----|-----------|
| #44 | arms/legs cost used `F_HANDS` not `F_ARMS`/`F_LEGS` | adjacent int index, copy-pasted branch |
| #45 | flame-lance rebound used `P_AIR` not `P_FIRE` | adjacent int index, copy-pasted branch |
| #48 | wind school checked `gsn_water_ward` not `gsn_wind_ward` | interchangeable globals |
| #49 | gouge used `weapons[11]`=pierce; spikes train `weapons[2]`=stab | **wrong magic slot number** |

#49 is the poster child: `weapons[11]` vs the correct `weapons[2]` is invisible on
review, but `weapons[WP_PIERCE]` vs `weapons[WP_STAB]` is catchable on sight by
anyone who knows spikes deal stab damage.

**Behavior does not change.** `#define WP_CLAW 5` then `weapons[WP_CLAW]` is
byte-identical after preprocessing. This is a pure readability + guard-rail change.

## The verified mapping (the deliverable)

`weapons[]` slots, triple-confirmed against source:
- `sweapon_lookup()` name table — `src/commands/act_wiz.c`
- the `score`/`level` display labels — `src/commands/act_info.c:2820-2827`
- `MAX_WEAPONS == 13` — `src/include/merc.h:140`

Damage-type relationship (from `src/core/fight.c`): a weapon's slot index equals its
damage type minus `TYPE_HIT` — `weapons[dt - TYPE_HIT]`, `TYPE_HIT == 1000`. So
`dt == TYPE_HIT + WP_x`. Mazoku confirms: claws `dt += 5` → slot 5; spikes
`dt += 2` → slot 2 (`fight.c:300-302`).

| Slot | Name    | Proposed macro | Notes / who uses it |
|-----:|---------|----------------|---------------------|
| 0    | hit     | `WP_HIT`       | unarmed/base; Fist's core skill (caps at 500 w/ discipline). Dominant literal — most of `fist.c`, `fight.c` |
| 1    | slice   | `WP_SLICE`     | |
| 2    | stab    | `WP_STAB`      | Mazoku **spikes** register here (`M_SPIKES → dt+=2`); gouge scales off this (#49) |
| 3    | slash   | `WP_SLASH`     | |
| 4    | whip    | `WP_WHIP`      | |
| 5    | claw    | `WP_CLAW`      | Mazoku **claws** (`dt+=5`); rake scales off this |
| 6    | blast   | `WP_BLAST`     | |
| 7    | pound   | `WP_POUND`     | |
| 8    | crush   | `WP_CRUSH`     | |
| 9    | grep    | `WP_GREP`      | legacy Diku attack-type name; keep verbatim |
| 10   | bite    | `WP_BITE`      | |
| 11   | pierce  | `WP_PIERCE`    | |
| 12   | blow    | `WP_BLOW`      | |

`MAX_WEAPONS` (13) stays as the array bound.

## Scope of edits

**Rename literal sites only.** Computed indices are left untouched (they're already
readable and can't be a constant):
- keep as-is: `weapons[dt-1000]`, `weapons[dt-TYPE_HIT]`, `weapons[i]`,
  `weapons[wp]`, `weapons[chdt]`, `weapons[wield->value[3]]`
- rename: every `weapons[<digit>]` literal (~66 sites: `fist.c` heavy on
  `WP_HIT`; `fight.c`; `mazoku.c`; the 13-slot dumps in `act_info.c`,
  `act_wiz.c`, `save.c`).

**`powers[]` is out of scope for renaming** — it's already named per class
(`P_*`/`S_*`/`F_*`/`M_*`). Its only bare `powers[<digit>]` sites are the
serialization dump (`save.c`) and wiz set/show (`act_wiz.c`); the linter will
cover those, but no semantic renaming is needed.

### Hard constraint: rename only, never renumber
`weapons[]` (and `powers[]`, `stances[]`) are persisted **positionally** in player
files (`save.c` writes/reads slot-by-slot). Changing a slot's *number* would
silently corrupt every existing character. This change only introduces names for
the existing numbers — no reordering, no renumbering.

## The lint guard (new work in `tools/`, not `src/`)

Add `tools/lint-c.mjs` (Node ESM, consistent with the other `tools/*.mjs`). It
scans `src/*.c` and **exits non-zero** on:
1. any bare `weapons[<digit>]` literal (post-migration there should be zero);
2. (extensible) the other footgun patterns from this session — e.g. a per-school
   damage case referencing a mismatched `gsn_*_ward` (#48), and loops bound at
   `MAX_*` inclusive (#47). Start with #1; leave hooks for the rest.

This is what makes the sweep *stick*: once complete, any new bare index is a
CI/pre-commit failure, not a bug we find months later by audit. Wire it into
`package.json` as `lint:c` and mention it in `CLAUDE.md`'s src/ conventions.

## Approach

1. Add the `WP_*` `#define` block to `merc.h` next to `MAX_WEAPONS`, in slot order,
   with a comment pointing at `sweapon_lookup()` as the source of truth.
2. Mechanical substitution of literal sites (scripted, then eyeballed per file —
   the diff must be name-for-value with **no value changes**; verify with a
   preprocessor/compile diff or `git diff -w` sanity read).
3. Add `tools/lint-c.mjs` + `lint:c` script; run it (must pass, zero bare indices).
4. Dev gate: `docker compose build mud` must compile (32-bit Debian). Smoke-test
   in dev — `score`/`level` weapon rows unchanged; a Mazoku rake/gouge and a Fist
   attack still behave identically (no numeric change expected).

## Acceptance criteria
- [ ] `WP_HIT..WP_BLOW` defined (0–12) in `merc.h`, order matches `sweapon_lookup`.
- [ ] Every literal `weapons[<digit>]` in `src/` replaced with its `WP_*` name;
      computed indices untouched.
- [ ] No slot **renumbered**; `save.c` format byte-identical; an existing player
      file loads with unchanged weapon rows.
- [ ] `tools/lint-c.mjs` reports zero bare `weapons[<digit>]` and is wired as
      `pnpm lint:c`.
- [ ] `docker compose build mud` compiles; dev smoke test shows no behavior change.

## Risk
Very low. Pure preprocessor substitution; the only real hazards are (a) a typo
mapping a name to the wrong number — mitigated by the verified table above and a
value-preserving diff review — and (b) accidentally renumbering — explicitly
forbidden and checked by the save-compat criterion. Large diff, but single-purpose
and mechanical.

## Follow-on (not this issue)
The same "name the magic index" treatment could extend to `stances[]` and to the
`do_runeweave` numeric `case` labels (rune bit-combos), and the linter can grow
the #47/#48 patterns. Track separately if wanted.
