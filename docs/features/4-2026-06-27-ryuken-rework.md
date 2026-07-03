---
issue: 4
type: balance
status: in-progress
opened: 2026-06-27
reporter: ScathachMDG
epic: 1
---

# Ryuken — overall buff + interactive interrupts (#4)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/4 · Epic: #1

## Summary
Ryuken (Saiyan dragon uppercut) was too lag-punishing to be worth it, and interrupted a
Sorcerer's chant unconditionally. Buff it overall, make the Sorcerer interrupt interactive
(Defense blocks the chant-break but still takes damage), reward hitting prepared targets, and
give it cross-class utility so it's useful outside the Sorcerer matchup.

## Current behavior (verified — `src/classes/saiyan.c:1029 do_ryuken`)
WAIT_STATE 42; physical `dice(body, body*3/2)`, ki `dice(spirit, spirit)`; vs Fist strip
Figure Eight + zero F_KI; vs Saiyan strip kiwall; vs Sorcerer unconditional `lose_chant`.

## Implemented design
- **Lag:** WAIT_STATE 42 → **28**, or **24** if the hit broke a major defense/charge (`broke_major`).
- **Damage:** physical `dice(body, body*2)`, ki `dice(spirit, spirit*3/2)`.
- **+15% bonus** if the target is in a major setup state (Defense / kiwall / any Patryn ward /
  Fist Figure Eight / Mazoku charge / Patryn block), detected *before* stripping.
- **vs Sorcerer:** if Defense up → strip Defense, **no** `lose_chant`, still deal damage; else
  `lose_chant` as before.
- **vs Fist:** strip Figure Eight + zero F_KI (unchanged).
- **vs Saiyan:** strip kiwall (unchanged).
- **vs Patryn:** strip one active block (Air > Fire > Negative); else drain one active ward by
  500 (`dec_duration`), priority Spirit > Earth > Flame > Wind > Water > Negative.
- **vs Mazoku:** if charging (`M_CTYPE != 0`), clear `M_CTYPE`/`M_CTIME` (scatter the charge).

## Acceptance criteria
- [ ] Lower lag (28, or 24 when it breaks something); damage noticeably higher than before.
- [ ] vs Sorcerer with Defense: Defense gone, chant survives, damage still lands.
- [ ] vs Patryn: a block is removed, or (no block) a ward loses 500 duration.
- [ ] vs Mazoku mid-charge: charge is scattered.
- [ ] +15% applies only when the target was in a setup state.

## Risk & balance notes
- Numbers (28/24 lag, body*2 / spirit*3/2, +15%, ward drain 500) are the reporter's proposal — tune in playtest.
- **Gap:** the "+15% vs *chanting*/Saiyan-*charging*" part of the spec is **not** detected — there's
  no clean in-progress-chant / Saiyan-charge flag to read. Implemented the reliably-detectable
  states instead (Defense/ward/kiwall/figure-eight/Mazoku-charge/Patryn-block). Revisit if a
  chant/charge state flag is added.

## Out of scope
- Adding new state flags for chant/charge detection (see gap above).
