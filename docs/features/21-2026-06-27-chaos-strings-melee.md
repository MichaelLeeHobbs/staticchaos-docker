---
issue: 21
type: balance
status: spec-review
opened: 2026-06-27
reporter: ScathachMDG
epic: 1 (follow-up)
---

# Chaos Strings — generic weapon-melee suppression (#21)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/21

## Summary
Chaos Strings applies `AFF_CHAOS_STRING` (#6), but the fight code only acts on it for **Fist**
(halves Fist melee) and **NPC** attack-count — so it does nothing to a non-Fist PC. Add a generic
weapon-melee reduction so the peel works against all melee.

## Reporter's decision (Skatha)
- Generic melee-damage reduction of **15%** (cautious) for `AFF_CHAOS_STRING` bearers.
- **Either/or with Fist:** Fist keeps its existing **−50%**; do **not** stack the 15% on top.

## Current behavior (verified — `src/fight.c:929`)
```
if ( IS_CLASS(ch,CLASS_FIST) )
  if ( IS_AFFECTED(ch,AFF_CHAOS_STRING) )
    if ( <melee dt> ) dam /= 2;     /* -50%, Fist only */
```
calc_attacks (~3539) also cuts attack count, but only for NPCs.

## Proposed design
In the same block, add an `else if` for **non-Fist** bearers:
```
if ( IS_CLASS(ch,CLASS_FIST) ) { ...existing -50%... }
else if ( IS_AFFECTED(ch,AFF_CHAOS_STRING) && <same melee dt range> )
  dam -= dam * 15 / 100;           /* -15%, all other classes */
```
Because Fist takes the `if` branch and everyone else the `else if`, the two never stack (either/or,
per Skatha). Apply to the same weapon-melee `dt` ranges the Fist case uses (TYPE_HIT..TYPE_HIT+15);
do not touch ranged/chant damage.

## Acceptance criteria
- [ ] A non-Fist PC under Chaos Strings deals ~15% less weapon-melee damage.
- [ ] A Fist under Chaos Strings still deals −50% (not −65%).
- [ ] Ranged/chant/other damage unaffected; no double-application.

## Risk & balance notes
- 15% is deliberately cautious (Skatha); tune in playtest. Contained to the one fight.c block.

## Out of scope
- The NPC attack-count behavior (unchanged); Chaos Strings damage/duration (unchanged).
