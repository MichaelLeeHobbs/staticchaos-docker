---
issue: 36
type: balance
status: spec-review
opened: 2026-06-28
reporter: ScathachMDG
---

# Ryuken — tune down (#36)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/36

## Summary
The #4 rework overshot — Ryuken is now too strong. Lower damage, raise wait, drop the setup bonus
to +10%. Playtest follow-up to #4.

## Reporter's decision (Skatha)
- **WAIT_STATE:** normal **30**, breaks-major **26** (not 42, not 24).
- **Damage:** physical `dice(body, body * 3 / 2)`; ki follow-up `dice(spirit, spirit * 5 / 4)`.
- **Setup bonus:** **+10%** (not +15%); applies only if target is chanting, charging, Ki Walled,
  Figure Eighted, Patryn warded, or Mazoku charging.

## Current state (verified — do_ryuken, saiyan.c)
- WAIT: `WAIT_STATE( ch, broke_major ? 24 : 28 )` (1169).
- Physical: `dam = dice( body, body * 2 )` (1155); Ki: `dam = dice( spirit, spirit * 3 / 2 )` (1163).
- Setup: `dam += dam * 15 / 100` on both hits (1157, 1165).
- Setup detection (1076–1093): gsn_defense, gsn_kiwall, **all six wards**, Fist figure-eight,
  Mazoku charge (M_CTYPE), **Patryn blocks** (air/fire/negative), Sorcerer mid-chant.

## Proposed change (the clear parts)
- `WAIT_STATE( ch, broke_major ? 26 : 30 );`
- `dam = dice( ch->pcdata->body,   ch->pcdata->body   * 3 / 2 );`
- `dam = dice( ch->pcdata->spirit, ch->pcdata->spirit * 5 / 4 );`
- both `dam += dam * 10 / 100;`

## Decision for review (maintainer) — setup trigger list
Skatha's list (chanting, charging, Ki Walled, Figure Eighted, Patryn **warded**, Mazoku charging)
is *narrower* than the current detection, which also fires on **Patryn blocks** and the **Sorcerer
Defense** barrier. Two things to settle:
- **"charging"** is ambiguous — Mazoku charging is already listed; is "charging" a second state
  (Saiyan ki-charge? Sorcerer Defense?) or just a restatement?
- **Keep or narrow:** the current set already covers every state she named; narrowing to *exactly*
  her list would drop the bonus vs Patryn blocks + Sorcerer Defense.
- **Recommendation:** keep the current (broader, comprehensive) detection and only change the
  percentage to 10% — it already includes all her named states and avoids inconsistent gaps. Confirm,
  or tell me which states to drop.

## Acceptance criteria
- [ ] WAIT 30 normal / 26 on a major break.
- [ ] Physical `dice(body, body*3/2)`, ki `dice(spirit, spirit*5/4)`.
- [ ] Setup bonus +10% on both hits.
- [ ] Strip/break/interrupt behavior unchanged; setup detection per the maintainer's call above.

## Risk & balance notes
- Straight tune-down of #4; numbers are Skatha's. Contained to do_ryuken.
