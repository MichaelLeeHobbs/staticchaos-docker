---
issue: 27
type: balance
status: spec-review
opened: 2026-06-27
reporter: ScathachMDG
---

# Balus Wall vs Gaav Flare (#27)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/27

## Summary
Make Balus Wall partly defend against Gaav Flare (it does nothing today). Gives Patryn (and
Sorcerer) a counter to Gaav Flare + a window to interrupt Holy Resist. **Two asks; only one needs
code** — see Holy Resist note.

## Reporter's decisions (Skatha)
- Balus Wall vs Gaav Flare: **reduce damage 50%**, **consume the wall**, **prevent Scorched**, do
  **not** fully absorb.
- Balus Wall is cast by the **Fire + Protection runeweave** (Patryn) *and* the Sorcerer chant — both
  should behave the same. (Resolved my earlier snag: Patryns **do** get Balus Wall.)
- Holy Resist: make it a 2-line interruptible complex chant, mutually exclusive with other complex
  chants / Laguna Blade.

## Verified in code
- **Balus Wall is one shared affect** (`gsn_balus_wall`): applied by the Patryn **FIRE+PROTECTION**
  runeweave (`patryn.c` case 1028) and by `chant_balus_wall` (Sorcerer). fight.c fully absorbs
  `SCHOOL_FIRE` for a wall holder (fight.c:1554) — but **Gaav Flare is `SCHOOL_BLACK`**, so it
  bypasses that block entirely (hence "does nothing today"). One change keyed on the affect covers
  both casters.
- **Gaav Flare** (`chant_gaav_flare`, sorcerer.c): rolls dam → `chant_damage(ch,victim,dam,cn)` →
  then applies the **Scorched** debuff itself (in the chant, after the damage), on a failed save.
- **Holy Resist is ALREADY a 2-line complex chant.** Its `chant_table` entry (sorcerer.c:3171) is
  `lines = 2` with line1+line2 set, so it's already interruptible and already blocked from coexisting
  with another complex chant or Laguna Blade by the `lines > 1` guards (sorcerer.c:264, :270).
  **No code change needed for the Holy Resist ask** — it already does what's requested. (My earlier
  "it's 1-line/instant" comment was wrong.)

## Proposed design (Balus Wall vs Gaav Flare)
Handle it inside `chant_gaav_flare` (localized; naturally covers damage, consume, **and** the
Scorched skip in one place; works for either caster of the wall):

```c
bool balus_blocked = FALSE;
...
if ( victim != NULL && is_affected( victim, gsn_balus_wall ) )
{
  dam -= dam / 2;                       /* 50% reduction, NOT full absorb */
  affect_strip( victim, gsn_balus_wall );   /* consume the wall */
  balus_blocked = TRUE;
  act( "$N's Balus Wall flares and smothers half the dark flame, then shatters!", ch, NULL, victim, TO_CHAR );
  act( "Your Balus Wall flares and smothers half the dark flame, then shatters!", ch, NULL, victim, TO_VICT );
}
... saves_chant / chant_damage( ch, victim, dam, cn ) ...
/* Scorched: also require !balus_blocked */
if ( !saved && !balus_blocked && victim != NULL && victim->position > POS_DEAD )
  ...apply scorched...
```

The existing `SCHOOL_FIRE` full-absorb (fight.c:1554) is untouched — regular fire attacks still get
fully eaten; only Gaav Flare (black) gets the new partial interaction.

## Acceptance criteria
- [ ] Gaav Flare on a Balus Wall holder: ~50% damage, wall consumed (stripped), **no** Scorched, with a message.
- [ ] Works whether the wall came from the Patryn FIRE+PROTECTION runeweave or the Sorcerer chant.
- [ ] Gaav Flare on a non-holder: unchanged (full damage, Scorched on failed save).
- [ ] Regular fire-school attacks vs Balus Wall: still fully absorbed (unchanged).
- [ ] Holy Resist: confirm in-play it interrupts as a 2-line chant (already configured) — no code expected.

## Risk & balance notes
- Small, localized to `chant_gaav_flare`. 50% + consume is the reporter's number; tune in playtest.
- If Holy Resist *feels* instant/uninterruptible in play despite `lines=2`, that's a **separate bug**
  to investigate — not part of this change.

## Out of scope
- Holy Resist (already a 2-line complex chant); any broader Balus Wall rework.
