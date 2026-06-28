---
issue: 3
type: balance
status: in-progress
opened: 2026-06-27
reporter: ScathachMDG
epic: 1
---

# Patryn Wind/Destruction — interactive interrupt (#3)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/3 · Epic: #1

## Summary
Patryn Air+Destruction was a flat **100% `lose_chant`** vs Sorcerer. Keep it a real
anti-caster tool, but make it respect Sorcerer defensive prep: Defense blocks the
interrupt (gets stripped instead), and without Defense it's a high-but-not-certain roll
reduced by the Sorcerer's resist buffs.

## Current behavior (verified — `src/patryn.c` case 513, AIR+DESTRUCTION)
vs Fist: strip Figure Eight, −1 F_KI. vs Sorcerer: **`lose_chant(victim)` unconditionally**.
Always: strip kiwall. (Reworked only the Sorcerer branch.)

## Implemented design
- **Sorcerer has Defense** (`is_affected(victim, gsn_defense)`): `affect_strip` Defense, **no**
  `lose_chant`; messages to both.
- **No Defense:** interrupt `chance = 75`, −10 each for `AFF_HOLY_RESIST`, `AFF_VAS_GLUUDO`,
  and (`AFF_RAYWING` or `AFF_WINDY_SHIELD`); clamped `URANGE(45, chance, 85)`. `number_percent()
  <= chance` → `lose_chant`; else a "chant unbroken" message. Fist/kiwall behavior unchanged.

## Acceptance criteria
- [ ] vs Sorcerer with Defense up: Defense is removed, chant survives (no `lose_chant`).
- [ ] vs Sorcerer without Defense: interrupts ~75% baseline, lower with resist buffs, never
      below 45% / above 85%.
- [ ] Fist (Figure Eight / F_KI) and Saiyan (kiwall) interactions unchanged.

## Risk & balance notes
- Numbers (75 base, −10 each, 45–85 clamp) are the reporter's proposal; tune in playtest.
- Contained to the case-513 Sorcerer branch.

## Out of scope
- Other runeweave combos; the optional "chant delay/stumble" on a failed roll (left as a plain message).
