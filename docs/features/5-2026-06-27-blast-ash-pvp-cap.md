---
issue: 5
type: balance
status: spec-review
opened: 2026-06-27
reporter: ScathachMDG
epic: 1
---

# Blast Ash — PvP safety cap (#5)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/5
Epic: #1 · Second of the two Sorcerer power cuts (Dynast Breath #2 shipped).

## Summary
Blast Ash is a guaranteed execute on a successful hit-roll — it deals damage equal to the
target's **entire current HP**, so it kills from full health. Keep the execute *flavor* but
**cap PvP damage so it can't kill above 25% HP**; true execute only kicks in once the target
is already low. PvE stays as-is.

## Reporter's request
"Blast Ash can create instant-kill behavior that is too swingy in PvP." Cap so it cannot kill
a high-HP target; allow execute at/below 25% HP. Numbers are the target feel (balance-pass OK).

## Current behavior (verified — `src/classes/sorcerer.c:760 chant_blast_ash`)
- Rolls a hit chance: `rank + will/2 (+ bonus >40)`, minus target defense
  (`-level*2/3` vs NPC, `-hit/150` vs PC; NPC level >95 → chance 0).
- On a failed roll: resisted, no effect.
- **On a successful roll: `chant_damage( ch, victim, victim->hit + dice(2,5), cn )`** — i.e.
  damage = current HP + 2–10 → a guaranteed kill regardless of the target's HP. This is the
  instakill.

## Proposed design
Keep the hit-roll (the save/resist gate is unchanged). Only change the **damage** step, and
only in **PvP** (`pvp = !IS_NPC(victim)`):

- **Target at or below 25% HP** (`victim->hit * 4 <= victim->max_hit`): **execute allowed** —
  `victim->hit + dice(2,5)` (current behavior).
- **Target above 25% HP:** capped, non-lethal:
  - `dam = victim->hit / 5 + rank * 20`  (= 20% current HP + rank×20; rank 50 → 20% + 1000)
  - `dam = UMIN( dam, victim->hit - 1 )`  — **guarantees it can't kill above 25%**, even a
    low-max-HP target (honoring "cannot directly kill unless below 25%").
- **PvE** (`IS_NPC(victim)`): unchanged (`victim->hit + dice(2,5)`).

Implementation note: this is a small change to the single `chant_damage(...)` line at the end
of `chant_blast_ash` (compute `dam` per the above, then one `chant_damage` call). The added
`UMIN(dam, hit-1)` clamp is mine — it makes the "no kill above 25%" rule airtight rather than
relying on the formula staying below the target's HP.

## Acceptance criteria
- [ ] vs a PC above 25% HP: Blast Ash (on a successful roll) deals the capped amount and **never
      reduces them below 1 HP** (no kill). Verify at high and low absolute max-HP.
- [ ] vs a PC at/below 25% HP: execute still works (can kill), gated by the existing roll.
- [ ] vs an NPC: damage unchanged (full-HP execute behavior preserved).
- [ ] The hit-chance/resist roll is unchanged in all cases.
- [ ] Dev verification: cast on a PC dummy at ~100% and at ~20% HP, and on an NPC; confirm the
      damage/kill outcomes and that the resist path still fires.

## Risk & balance notes
- **Numbers need your balance pass** (reporter agreed): the cap `20% current + rank×20` and the
  25% execute threshold are the levers. Confirm or adjust.
- Contained: only the damage computation in `chant_blast_ash` changes; the roll and PvE path
  are untouched, so blast radius is minimal.
- Independent of Dynast Breath (#2) and the counterplay patches (#3/#8) — safe to ship/playtest
  on its own.

## Out of scope
- The hit-chance/resist formula (unchanged).
- Other Sorcerer chants.
