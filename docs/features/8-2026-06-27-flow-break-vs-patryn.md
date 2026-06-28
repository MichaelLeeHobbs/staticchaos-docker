---
issue: 8
type: balance
status: spec-review
opened: 2026-06-27
reporter: ScathachMDG
epic: 1
---

# Flow Break vs Patryn wards/blocks (#8)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/8 · Epic: #1
Last patch in the epic; pairs with the now-shipped counterplay (#3/#4).

## Summary
Give Sorcerer's Flow Break an explicit, prioritized interaction with Patryn defenses so it's a
real answer to prepared wards/blocks — **save-scaled** (a save drains; a failed save removes).

## Current behavior (verified — `src/sorcerer.c:1196 chant_flow_break`)
Flow Break already: messages; collapses/weakens a Holy-Resist barrier; strips Vas Gluudo; shatters
Laguna Blade; disrupts Recovery; strips up to `strips = URANGE(1, 1+rank/18, 3)` temporary affects;
and on a **failed save** interrupts an in-progress Sorcerer chant (`saves_chant`, `pcdata->chant`).

**The gap:** Patryn **blocks** (`NEW_AIR_BLOCK` / `NEW_FIRE_BLOCK` / `NEW_NEGATIVE_BLOCK`) are
`pcdata->actnew` *flag bits*, not affects — so the generic strip loop **cannot remove them at all**.
Patryn **wards** (`gsn_*_ward` affects) can be caught incidentally by the generic strip, but there's
no prioritized/“drain vs remove” behavior the design wants.

## Proposed design
Add a **Patryn-specific branch** (when `IS_CLASS(victim, CLASS_PATRYN)`), gated by one `saves_chant`:

- **Saved:** drain one active ward's duration by **300** (`dec_duration`), and a **25%** chance to
  remove one active block.
- **Failed save:** remove **one** active block **or** (if no block) one active ward — block first.

Priority order (from #1):
- Blocks: **Air → Fire → Negative**
- Wards: **Spirit → Earth → Flame → Wind → Water → Negative** (`gsn_*_ward`)

Messaging on each outcome (attacker TO_CHAR, victim TO_VICT): ward drained, block torn, ward unravelled.

### Implementation notes (to resolve at build)
- **Avoid double-removal with the generic strip:** the existing `strips` loop (lines 1252-1264)
  can already remove a ward affect. Run the explicit Patryn branch first and have the generic loop
  **skip `gsn_*_ward` types** (and the blocks aren't affects, so they're unaffected) so a ward isn't
  both drained *and* stripped in one cast.
- **Save bookkeeping:** the Sorcerer-chant branch calls `saves_chant` only for Sorcerer victims, so a
  Patryn victim can take its own independent `saves_chant` roll without colliding.
- Blocks are removed with `REMOVE_BIT(victim->pcdata->actnew, NEW_*_BLOCK)`; wards drained via
  `dec_duration(victim, gsn_*_ward, 300)` and removed via `affect_strip(victim, gsn_*_ward)`.

## Acceptance criteria
- [ ] vs Patryn, saved: an active ward loses ~300 duration; ~25% of the time an active block is also removed.
- [ ] vs Patryn, failed save: exactly one block is removed (Air>Fire>Negative), or — if no block — one ward (priority order).
- [ ] A ward is never both drained and stripped by the same cast (no double-dip with the generic strip).
- [ ] Non-Patryn behavior (holy-resist/vas-gluudo/laguna/recovery/chant/generic strip) unchanged.
- [ ] Dev verification: cast vs a Patryn with a block up and with only a ward up, on save and on failed save.

## Risk & balance notes
- Numbers (300 drain, 25% block-on-save) are the reporter's proposal; tune in playtest.
- Contained to `chant_flow_break` plus a small skip-condition in its generic strip loop.

## Out of scope
- Reworking the generic strip or the chant-interrupt logic.
