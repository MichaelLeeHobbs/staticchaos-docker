# Class Trials — Phase B: Per-Class Objectives (research-backed design)

**Status:** DESIGN, for Skatha's review. Not built. Supersedes the "shared level-scaled gauntlet"
that ships today (Phase A). Grounded in a deep-research pass (sources at the bottom); where the
research doesn't cover text-MUD specifics, that's flagged.

## The core shift (what the research says)
Today every tier is the **same mob scaled by level** — it tests survival, not class skill. The
established fix, across MMOs/RPGs/MUDs, is **per-class objectives that make the player perform their
class's signature mechanic inside a designed encounter.** Four findings drive the whole plan:

1. **Per-class/per-role objectives, not a generic fight.** WoW's *Proving Grounds* is the canonical
   model: single-player scenarios split by role with mechanically distinct goals (DPS = kill priority
   targets on a timer; Tank = protect an NPC from waves; Healer = keep 5 named allies alive through
   escalating damage). EverQuest gives every class its *own* epic quest/weapon; TorilMUD's only remort
   (Lich) is Necromancer-gated. Advancement content tied to a class is a long, memorable lineage. *(high)*
2. **Escalate by complexity, not stats.** Proving Grounds tiers (Bronze→Silver→Gold→Endless) add
   *more enemy types and combinations*, not bigger numbers; "intensity = how often the player must
   re-plan." Bullet-sponge stat inflation is explicitly the weak pattern we have now. *(high)*
3. **Teach one mechanic at a time — show, don't tell.** Introduce each mechanic in isolation, low
   threat, **telegraphed/easy to react to**, in order of importance; *then* combine it with others.
   (Mario's lone Goomba; Portal's progressive chambers.) *(medium)*
4. **To FORCE a mechanic, constrain alternatives + saturate with opportunities, and re-test it every
   tier or players forget.** (Half-Life 2 Ravenholm starves other ammo so you *must* use the gravity
   gun.) The MUD version: a foe that **can only be beaten by the class mechanic** (immune/heals/enrages
   otherwise). *(medium)*

Reward stays a **hybrid** (XP + Primal): speeds early progression without becoming pure skill-gating.
Use a gradual/adaptive curve, **actionable failure feedback**, and fair scaling. *(medium)*

## The key adaptation the research does NOT give us: telegraphs in text
Every "force a defensive/interrupt response" lesson in the sources assumes a *visual* telegraph (a
wind-up animation). A text MUD has none. **So the central new primitive is a text wind-up:**

> A trial foe **announces a multi-tick wind-up** via room echoes, and the player must perform the
> required class action **before the wind-up resolves on a later pulse.**

Example (Sorcerer, "protect your chant" tier):
```
The wraith's eyes flare. 'I will silence your prattling!'   (tick 0 — telegraph)
The wraith gathers a wave of disrupting force...            (tick 1 — last chance)
-> if the player raised Defense/Wind/Vas Gluudo: "Your shield drinks the wave — your chant holds!"
-> else: the wraith's wave lands; the player's in-progress chant is interrupted (lesson delivered).
```
This single mechanic — *telegraph → react-window → success/failure with a teaching message* —
implements almost every per-class objective. Failure must always **say what to do next time**
(actionable feedback), since text has no other channel.

## Recommended build: one trial engine, class/tier "objective overlays" (not 40 bespoke fights)
40 hand-authored encounters (5 classes × 8 tiers) is unmaintainable. Instead build **one reusable C
"trial encounter" spec** parameterized by an *objective descriptor*:
- `required_action` — what the player must do this fight (e.g. CAST_UNDER_PRESSURE, INTERRUPT_WINDUP,
  SURVIVE_BURST_WITH_DEFENSE, POWERED_UP_DAMAGE_ONLY, USE_RUNEWEAVE, SCHOOL_SWITCH, PRIORITY_TARGET).
- `windup_telegraph` / `success_msg` / `fail_effect` text.
- `gate` — the Ravenholm constraint (foe immune / self-heals / enrages unless the action is performed).
- `win_condition` — kill / survive N wind-ups / keep an NPC alive / kill priority target first.

This is the research's "shared skeleton + class-specific overlays" answer (cheaper, consistent feel,
re-tests the mechanic every tier). The damage-gating bits (e.g. "only takes damage while powered up")
**need C** (a hook in `damage()` or a `spec_` function) — mobprogs can't gate damage; but the
telegraph/objective text and many checks can reuse mobprog `fight_prog` + our new `carries` ifcheck +
`isaffected`. Expect to add a small `spec_trial` family in `special.c` plus 1-2 helper ifchecks
(e.g. "is the actor currently power-charged / mid-chant").

## Per-class curricula (8 tiers each: introduce → combine → PvP/raid-prep)
These are a **starting curriculum** — Skatha (the class expert) should sanity-check each class's
actual ability names/mechanics; the research explicitly notes no source models a 5-class MUD directly,
so class accuracy needs his eyes. Pattern per class: T1-3 introduce core mechanics in isolation; T4-6
combine/pressure; T7-8 mock-PvP against a dummy of the class's hardest matchup.

**Sorcerer** (fragile chant-caster; signature = landing chants under interrupt pressure, school choice, debuffs)
1. *Easy* — Land any chant on a passive dummy (cast → resolve). 2. *Easy-Med* — Cast through chip damage from a weak attacker. 3. *Med* — **Telegraph**: foe winds up a big hit; raise Vas Gluudo/Defense/Wind first to survive. 4. *Med-Hard* — Foe winds up a self-heal; **strip/disrupt it** (Diem Wing/Flow Break) or it heals to full. 5. *Hard* — Two foes, one Black-resistant, one White/Astral-resistant: **switch schools** to hurt each. 6. *Hard* — Survive a telegraphed nuke with the **right resist up** (Holy Resist vs black). 7. *Very Hard* — Land a long chant **while** interrupting the foe's counter-cast and holding a defense. 8. *Extreme* — Dummy that **Ki-Waves on a telegraph** (the Saiyan matchup): protect the chant and win.

**Saiyan** (Ki resource, Ki Wall, power-up, Ki Wave interrupt)
1. Foe takes real damage **only while powered up** (forces the power-up). 2. **Telegraph**: raise Ki Wall before a big hit. 3. **Ki Wave to interrupt** a telegraphed enemy cast. 4. Ki management — sustain power-up + Ki Wall across a long fight without running dry. 5. Adds: kill the **priority** caster first (via Ki Wave). 6. Survive a burst with Ki Wall *timing* (telegraph → wall → punish window). 7. Combine power-up burst + Ki Wall + Ki Wave under pressure. 8. Mock-PvP vs a chanting dummy: interrupt + close it out.

**Fist** (Ki/spirit martial arts, dim-mak interrupt, kicks)
1. Build Ki, land basic kicks. 2. Survive a telegraphed hit via defensive stance. 3. **Dim-mak to interrupt** a telegraphed cast. 4. Kick **combo windows** (timing). 5. Priority target / adds. 6. Survive burst. 7. Combine combo + interrupt under pressure. 8. Mock-PvP vs caster dummy.

**Patryn** (rune magic, two-rune runeweave combos, rune strength)
1. Cast a single rune effect. 2. Use a **runeweave combo** vs a foe immune to single runes. 3. Pick the **right combo** for a resistance. 4. **Raise rune strength** mid-trial to break a threshold. 5. Two foes needing different combos. 6. Survive a burst with a defensive runeweave. 7. Combine combos under pressure. 8. Mock-PvP.

**Mazoku** (astral/demonic powers, release-blast interrupt)
1. Land a basic astral power. 2. Survive a telegraphed hit. 3. **Release blast to interrupt** a telegraphed cast. 4. Astral resource/positioning management. 5. Priority target. 6. Survive burst. 7. Combine under pressure. 8. Mock-PvP.

## Difficulty & reward tuning
- **Escalate by complexity**: add a second foe / a second simultaneous wind-up / a phase change, not
  raw hp. Keep even Tier 1 *demanding* by requiring the mechanic to even start dealing damage.
- **Pass/fail with feedback**: every failed wind-up prints the corrective ("Too slow — raise Ki Wall
  *before* the blow"). This is the only teaching channel text has.
- **Rewards**: keep the existing XP+Primal ramp (Phase A), but consider weighting it so the *mechanic*
  tiers (T3-4, where the lesson lands) pay a bit more — reward learning, not just survival. Preserve
  the "clear the basics ≈ Eval 10 / 2nd class" target so trials speed the early game without
  outpacing the endgame economy.

## Open decisions for Skatha
1. Per-class ability accuracy: confirm/adjust each class's signature mechanic + the exact spell/skill
   names the tiers should force (I inferred from this session's code).
2. Bespoke-per-tier vs the shared trial-engine + overlays (recommended). OK to build the engine first
   and ship Sorcerer + Saiyan as the pilot pair, then the other three?
3. Telegraph window length: how many ticks of warning before a wind-up resolves (reaction fairness)?
4. Should T8 mock-PvP dummies mimic the class's *worst matchup* (most useful prep) or be generic?

## Sources (deep-research, adversarially verified)
- WoW Proving Grounds — per-role objectives, tiered complexity: https://warcraft.wiki.gg/wiki/Proving_Grounds *(high)*
- Encounter pacing / "intensity = re-plan", enemy-palette escalation: https://www.gamedeveloper.com/design/the-art-and-science-of-pacing-and-sequencing-combat-encounters ; https://book.leveldesignbook.com/process/combat/encounter *(high)*
- Teach-via-design / show-don't-tell / one-at-a-time: https://remptongames.com/2018/09/22/no-more-tutorials-how-to-convey-information-through-design/ ; https://www.gamedeveloper.com/design/super-mario-bros-3-level-design-lessons-part-1 ; https://www.gamedeveloper.com/design/teaching-game-mechanics-a-hierarchy-of-learning *(medium)*
- Telegraphs / wind-ups pose the challenge-question: http://www.chaoticstupid.com/enemy-attacks-and-telegraphing/ *(medium)*
- Per-class quest lineage: https://wiki.project1999.com/Class_Epic_Quest_List ; https://muds.fandom.com/wiki/TorilMUD *(high)*
- Difficulty curve / feedback / scaling (CLT): https://www.intechopen.com/chapters/1221745 *(medium)*

**Caveats from the research:** sources skew to design blogs + one open-access academic chapter (no
controlled studies) — fine for craft principles, corroborated across sources. WoW Proving Grounds is
by *role* (3 buckets), not 5 individual classes, so the differentiation lesson transfers but our
granularity is finer than any cited system. **No source models a text-MUD telegraph or a 5-class
room-gated trial ladder** — the tick-based wind-up adaptation above is our own and should be
playtested for reaction fairness. "Telegraph forces the response" is more precisely "prompts" it.
