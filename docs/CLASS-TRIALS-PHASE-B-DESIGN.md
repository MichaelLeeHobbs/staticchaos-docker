# Class Trials — Phase B: Per-Class Objectives (research-backed design)

**Status:** IMPLEMENTED & DEPLOYED (2026-06-21). The engine + all 5 classes' 8-tier objective tables
are live (`src/world/trials.c`). Supersedes the Phase A "shared level-scaled gauntlet." Skatha's review
decisions are folded in (see "Decisions"). Grounded in a deep-research pass (sources at the bottom).

## Implementation status (what shipped)
- **Engine:** `src/world/trials.c` — a data-driven `trial_table[MAX_CLASS][MAX_TRIALS]` of objective
  descriptors + `trial_ready()` (resolves each requirement from live state) + `trial_damage_gate()`
  (you can only damage the tier mob while in the required state) + `trial_update()` (the stateless,
  time-synced telegraph: wind-up echo, then a scaled punish + corrective if you're not ready). Hooks:
  `damage()` (fight.c, the gate), `second_update()` (update.c, the clock), `do_quest_trial()`
  (quest.c, the lesson-on-entry). `trials.o` added to the Makefile.
- **Requirement matrix (per class × tier 1-8, tunable):**
  Saiyan: NONE, POWERUP, KIWALL, POWERUP, KIWALL, KIWALL, POWERUP, KIWALL ·
  Sorcerer: NONE, DEFENSE, VAS GLUUDO, DEFENSE, HOLY RESIST, HOLY RESIST, DEFENSE, DEFENSE ·
  Patryn: NONE, DEFENSES, WARD, DEFENSES, WARD, WARD, DEFENSES, WARD ·
  Fist: NONE then FIST-KI ·  Mazoku: NONE then FOCUS.
- **Verified live:** entry lesson, damage-gate (both directions), telegraph fire, scaled punish +
  corrective — for Saiyan POWERUP and Sorcerer DEFENSE. Other requirements share the same
  table-driven code path (compiled clean; not each individually driven).
- **Known simplifications (best-judgment, tunable — owner approved "change later"):**
  (1) "active-action" tiers from the curricula (interrupt / school-switch / runeweave-choice) currently
  gate on the closest *persistent-state* proxy rather than detecting the exact ability use; upgrading
  to true per-ability detection is a clean later add. (2) POWERUP requires S_POWER ≥ ⅓ of max so a
  resting Saiyan doesn't count as powered-up. (3) The exact ability names per tier are best-judgment.
- A `-Woverflow` truncation bug in the `IS_AFFECTED` high-bit checks (AFF_VAS_GLUUDO / AFF_HOLY_RESIST)
  was found and fixed during the build.

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

> A trial foe **announces a wind-up** via a room echo, and the player must perform the required class
> action **within a tight, PvP-speed window** before the wind-up resolves.

**Timing (per Skatha): ~1-2 real seconds to react** — deliberately PvP-fast, not a leisurely turn.
The engine pulses 4×/sec (`PULSE_PER_SECOND`), so the wind-up resolves roughly **4-8 pulses** after
the telegraph. This must run on the per-pulse loop, **not** the 3-second combat round, so it can
demand sub-round reactions the way real PvP does (e.g. interrupting a chant the instant it starts).

Example (Sorcerer, "protect your chant" tier):
```
The wraith's eyes flare. 'I will silence your prattling!'   (telegraph — clock starts, ~1-2s)
-> player raises Defense/Wind/Vas Gluudo in time: "Your shield drinks the wave — your chant holds!"
-> too slow: the wave lands; the in-progress chant is interrupted (lesson delivered + corrective text).
```
This single mechanic — *telegraph → tight react-window → success/failure with a teaching message* —
implements almost every per-class objective. Failure must always **say what to do next time**
(actionable feedback), since text has no other channel. The 1-2s window is the headline tuning knob;
playtest it for fairness (text latency + typing speed) and consider loosening early tiers slightly.

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

**Patryn** (rune magic; signature = knowing your **runeweave spells** and casting the right one)
*Correction per Skatha: a "runeweave combo" is just the **name** of a spell — each spell is identified
by a two-rune label (e.g. "air + life"); it is **not** fusing two elements. So the teaching goal is:
learn **which combo-name maps to which spell and what that spell does**, then pick the correct one for
the situation.*
1. Cast a basic runeweave spell (learn the combo → spell → effect chain). 2. A foe only a **specific** runeweave spell can hurt — produce that spell by its combo-name. 3. Read the situation and choose the **right** runeweave from several (the core skill). 4. **Raise rune strength** to push a runeweave past a damage/effect threshold. 5. Two foes, each beaten only by a different runeweave spell. 6. Survive a burst with a defensive runeweave. 7. Chain the correct runeweaves under pressure. 8. Mock-PvP vs the Patryn's hardest matchup.

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

## Decisions (resolved with Skatha, 2026-06-21)
1. **Runeweave understanding corrected.** A runeweave "combo" is just a spell's **name** (a two-rune
   label), not elemental fusion. Patryn trials teach *which combo-name = which spell + what it does*
   and choosing the right one. (Patryn curriculum above rewritten accordingly.)
2. **Pilot = Sorcerer + Saiyan first**, then Fist / Patryn / Mazoku — built on the shared trial-engine
   + per-class overlays (the recommended approach).
3. **Telegraph window = PvP speed: react within ~1-2 seconds** (≈4-8 pulses; runs on the per-pulse
   loop, not the combat round). Folded into the telegraph section above; it's the key tuning knob to
   playtest.
4. **T8 mock-PvP dummies = the class's hardest matchup** (teach players how to beat their worst
   matchup), not generic.

### Still to confirm at build time (per class, as we implement)
- The exact spell/skill names each tier should force (Skatha is the class expert — we'll confirm the
  precise ability names per class when we build that class, starting with Sorcerer + Saiyan).

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
