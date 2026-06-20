# Class Trials — Design Doc (for review before any implementation)

**Status:** DESIGN APPROVED by Skatha (2026-06-20). Still **not built** — this is a future phase, not
part of the current patch. The decisions below are now the authoritative spec; the "Open questions"
section at the bottom is resolved and kept only for history.

## Decisions (resolved with Skatha)
1. **Reward = XP (spendable) + Primal, NO leveling.** This game has no direct leveling — rank is
   derived from current stats, and XP is the currency you spend to raise stats. So trials simply
   **grant XP to spend** (plus Primal). This is effectively reward model **R2**: do NOT re-enable
   `advance_level`/auto-leveling; just credit `ch->exp` (the spendable pool) and Primal. Lower risk.
2. **8 tiers**, difficulties: Easy, Easy-Medium, Medium, Medium-Hard, Hard, Hard, Very Hard, Extreme.
   Target: completing the **basics** grants enough XP + Primal to reach roughly **Eval 10 / 2nd
   Class** — enough that a new player leaves the trials already knowing some abilities (not just
   auto-attack) and can start exploring. (This replaces the old "start with Primal" handout.)
3. **Per-class teaching.** Trials must teach the player's *current* class. Preferred: one shared trial
   framework parameterized by class (tier room + objective vary by `ch->class`); fall back to separate
   trial sets per class only if a shared framework can't express the class-specific lessons. (Per-class
   content is the larger cost — flag at build time.)
4. **Solo enforcement = `ROOM_SOLITARY` bounce + per-player owner-damage** (no true instancing —
   Skatha agrees instancing would disturb the code too much).

Rewards = **XP + Primal**. Nothing here is built yet.

## Goal
Solo, escalating class trials that *teach* a class and reward XP + Primal — enough that clearing the
basics replaces the old "start with Primal" handout and meaningfully speeds the grind to endgame/PvP.
A quest master near the start (arena right after MUD School). Even basic trials should demand real
mechanical understanding. Pair with buffing the weakest early mobs so nothing near spawn is zero threat.

## Hard constraints found in the code (the things that shape the design)
1. **No instancing.** Rooms are global singletons (`room_index_data`, merc.h:1675) — there is no
   per-player room copy and no on-entry hook. True instances would be a ~500-LOC engine refactor and
   are **not recommended**.
2. **Leveling/XP is currently OFF.** `gain_exp` (update.c:66) accumulates `exp`/`totalexp` but the
   level-up loop is commented out and `advance_level` (update.c:55) is a stub. So "reward XP" needs a
   decision: re-enable leveling, or grant XP/Primal as a tracked currency without auto-leveling.
3. **Quest masters already exist** as mobs flagged `ACT_PRACTICE`; `do_quest` (quest.c) is the
   natural home for a `quest trial` verb. Primal is already a tracked resource.
4. **Room privacy flags exist but are mostly disabled.** `ROOM_PRIVATE`/`ROOM_SOLITARY`
   (merc.h:1169) are referenced but `room_is_private()` (handler.c:1371) currently returns FALSE.
   They can be revived for gating.

## Recommended approach — "private trial zone", phased
Rather than fake instancing, use a **small dedicated trial area + entry gating + per-player target
mobs**. This is the realistic way to get "solo, non-interferable" on this engine.

### Phase A — MVP (single occupant per trial room)
- Build a `trials.are` area: a lobby + one room per trial tier.
- `quest trial <n>` at the quest master: validates prerequisites, then **teleports** the player into
  the tier room. On entry, enforce solo by reviving `ROOM_SOLITARY`: if another player is already in
  that room, bounce the newcomer to the lobby ("A trial is already in progress here.").
- Trial mobs are flagged so **only the assigned player** can damage/be-damaged by them (a
  `quest_owner` check in the damage path), so outsiders can't grief or leech even if they get in.
- On completion (all trial mobs dead / objective met): award XP + Primal, teleport back, mark the
  tier done on the player (a bitmask in pcdata).
- Limitation: shared mob state if two players force the same room — mitigated by the SOLITARY bounce
  + owner-damage check. Good enough for a friends' server.

### Phase B — polish
- One room per tier per a small pool (round-robin a few duplicate rooms) so several players can run
  trials at once.
- Timers/reset so an AFK player doesn't lock a room forever.
- Scaling: tier mobs scale from "prove you can use your core class buttons" → "survive a scripted
  burst" → "mini raid-boss mechanics / PvP-like dummy".

## Reward model (needs your decision)
Two options for granting XP given leveling is off:
- **(R1) Grant Primal + a flat XP/level bump per tier**, re-enabling a *bounded* version of
  `advance_level` only for trial completion (cleanest player-facing "I leveled").
- **(R2) Award Primal + bank XP** without auto-level (keeps the current "leveling off" stance);
  players spend/convert later. Lower risk, but "XP reward" feels less tangible.
Recommendation: **R1 with a hard cap** (trials can carry a new char to the old Primal-start power
level, no further), so trials replace the Primal handout without trivializing endgame.

## Early-game threat pass (the sub-request)
Separately and cheaply: bump the weakest MUD-School/early-Midgaard mobs so none are zero threat.
This is data-only (mob level/hp/damage in the relevant `.are` files) and can ship before trials.

## Open questions for Skatha
1. R1 (trials can level you, capped) vs R2 (XP banked, no auto-level)?
2. How many tiers, and what's the Primal/XP target for "cleared the basics"?
3. Per-class trials (5 separate sets) or one shared gauntlet that tests your current class? (Per-class
   is much more content.)
4. Acceptable that "solo" is enforced by bounce + owner-damage rather than true instancing?

## Effort
Phase A ≈ L (new area + entry gating + owner-damage hook + quest verb + reward grant). Phase B medium
on top. Early-mob threat pass ≈ S (data only). Not started — awaiting answers above.


## Responses
Skatha — 3:28 PM
That approach sounds like it will work.

1.) There's no direct leveling in the game it's rank based on current stats. You use the XP to raise stats. So it should just give the player XP to spend.

2.) There should be 8 tiers with the following difficulties: Easy, Easy-Medium, Medium, Medium-Hard, Hard, Hard, Very Hard, and Extreme. Completing the basics should give enough XP and Primal to get to Eval 10 or 2nd Class. The idea is that completing the basics will let the player start exploring already knowing some abilities so it's not just auto attack.

3.) The trials need to teach the specific class the player is using if that could be done without separate trials sets that's fine, but if not we'd need separate sets for each class.

4.) The solo enforcement proposed sounds fine. It seems like adding true instancing would mess with the code too much.

