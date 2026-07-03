---
issue: 23
type: balance
status: spec-review
opened: 2026-06-27
reporter: ScathachMDG
epic: 1 (follow-up)
---

# Holy Bless — white-specialist resist bonus + ally buff (#23)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/23

## Summary
Finish the deferred Holy Bless tweaks: a White-specialist resist bonus (the white-spec's reward),
and confirm Holy Bless stays castable on allies (White Sorcerer's one ally buff).

## Reporter's decision (Skatha)
- Implement the **white-specialist resist bonus**: while blessed by a white-specialist, +5% chance
  to resist **curse / blind / sleep / magical no-flee (root)**.
- **Keep ally-targeting** (castable on others) — do **NOT** add the PvP grouped-only restriction now;
  "can be tuned later if it becomes a problem when there are more players."

## Current state (verified)
- `chant_holy_bless` (`src/classes/sorcerer.c`) applies a `bless`-type affect (hit/dam) and already targets
  `vo`, so it can be cast on others today — nothing to add for the ally-buff part (and no PvP
  restriction per Skatha).
- White-specialist = `ch->pcdata->powers[SORC_SPEC] == SCHOOL_WHITE` (cf. sorcerer.c:556).

## Proposed design
The bonus must be **white-specialist-only**, but the `bless` affect doesn't record who cast it.
So mark it at cast and check the mark at each effect-application site:

1. **Mark:** when `chant_holy_bless` is cast by a white-specialist, apply a distinct marker affect
   on the target (e.g. type `skill_lookup("holy bless")` if present, else a dedicated gsn/bit) so
   it's distinguishable from a plain `bless`. Add a helper
   `bool holy_blessed_ward( CHAR_DATA *victim )` -> true if that marker is present.
2. **Resist hook (5%)** at each place these effects are *applied*, before applying: if
   `holy_blessed_ward(victim)` and `number_percent() <= 5`, skip applying + brief message. Sites:
   - **curse:** Patryn elemental curses in `src/classes/patryn.c` (gsn_*_curse application).
   - **blind:** `chant_lighting` (sorcerer.c) and `spell_blindness` / `AFF_BLIND` application (magic.c).
   - **sleep:** `chant_sleeping` (sorcerer.c) and `spell_sleep` (magic.c).
   - **no-flee/root:** `chant_dynast_breath` and laphas (sorcerer.c) AFF_NO_FLEE application.

   (Spec note from #6: "checked only where those effects are applied; does not need to show as raw
   saving throw" — so a small inline check at each site, not a global saving-throw change.)

### Decisions for review (maintainer)
- **Marker mechanism:** a dedicated marker affect/gsn is cleanest (recommended). Confirm whether to
  add a new skill_table entry "holy bless" for the marker type, or reuse a spare AFF_ bit.
- **Scope of sites:** the four effect families above — confirm the list (esp. which "curse"/"sleep"
  sources count).

## Acceptance criteria
- [ ] A target blessed by a **white-specialist** resists ~5% of incoming curse/blind/sleep/no-flee.
- [ ] A target blessed by a **non-white-specialist** gets the normal bless (hit/dam) but **no** resist.
- [ ] Holy Bless is still castable on allies (no new targeting restriction).

## Risk & balance notes
- Cross-cutting: touches several effect-application sites (patryn.c, sorcerer.c, magic.c) + a marker.
  Main risk is missing a site or the marker leaking onto a plain bless — keep the marker distinct.
- 5% is small/cautious; tune in playtest.

## Out of scope
- PvP ally-target restriction (explicitly deferred by Skatha).
