# Static Chaos — Server & Sorcerer Changes: Brainstorm

> Working design doc off Skatha's notes. **Every claim below was checked against the
> current source** (`src/sorcerer.c`, `update.c`, `interp.c`, `merc.h`). Skatha's findings
> are all accurate — file:line references are included so we can move straight to code.
> Nothing here is implemented yet; this is the "think it through + keep it balanced" pass.

---

## Implementation status (2026-06-09)

Implemented and deployed, in the recommended order:

| Done | Item | Commit-ish |
|:--:|------|------|
| ✅ | Stored-rank fix (resolve at captured rank) + misfire Mystic refund | slice 1 |
| ✅ | Target-fix bugs: holy_bless / lighting strip the **victim** now | slice 1 |
| ✅ | New players start with **125 primal** | slice 1 |
| ✅ | **Flow Break** implemented (the anti-magic counter) | slice 2 |
| ✅ | **saves_chant** retuned (base 45; floors 10/12/15; ceilings 75/85) | slice 3 |
| ✅ | **Laguna Blade** finisher (force-wield, Mystic drain, lockout, cleanup) | slice 4 |
| ✅ | **Recovery** immediate heal + cleanse (on top of the HoT); Laguna tick-expiry made safe | slice 5 |
| ✅ | **Non-destructive respec** (`sorc_rank` inactive-not-dropped; respec 250; off-spec cap 44) | slice 6 |
| ✅ | **chant list / chant info / research list** commands | slice 7 |
| ✅ | **pdelete / prename** immortal commands | slice 8 |
| ✅ | Help text: SORCERER, CHANT, MYSTIC, RESEARCH, SPECIALIZE, SAVES, FLOW BREAK, LAGUNA BLADE, RECOVERY | slice 9 |
| ⏸️ | **§3.6 partial-damage-on-save + per-rank lag bands** — *deliberately deferred* | — |
| ⏸️ | Laguna Blade "partial resist bypass" — *deferred* (damage-pipeline change) | — |

**Why §3.6 is deferred (and that's the balanced call):** it requires editing the
`saves_chant`-return pattern in ~30 individual damage chants and the lag/wait fields of
all 81 `chant_table` rows — a wide, error-prone sweep on a live server. Most of its intent
("early Sorcerer shouldn't whiff") is *already* delivered by the stored-rank fix (full-power
resolution) and the lower save floors (chants land far more often). The remaining polish
(half-damage instead of a total whiff) is best done as its own careful pass with Skatha
confirming which chants are damage vs hard-control. Recommend doing it after watching the
current changes in real fights.

---

## TL;DR — recommended order (balance-aware)

The changes are not independent. Several of them **buff Sorcerer at the same time**, so the
order matters or we overshoot from "too weak" straight to "oppressive."

1. **Pure bug fixes** that are also correctness wins: stored-rank fix, `holy_bless`/`lighting`
   target fixes, Recovery loop consolidation. *(The stored-rank fix is by itself a large power
   jump — treat it as a balance change, not a freebie.)*
2. **Counterplay first:** finish **Flow Break** + confirm anti-magic works *before* lowering
   save floors or enabling Laguna Blade. Otherwise disables/finishers have no answer.
3. **Reliability:** partial damage on save, lower lag on low-rank chants.
4. **Tuning dials:** `saves_chant` numbers, specialization caps/respec.
5. **Payoff content:** finish **Laguna Blade**.
6. **Recovery** rework.
7. **Docs + admin commands** — independent, low-risk, do anytime.

**Golden rule for this pass:** land the stored-rank fix *first and alone*, watch it, then tune
saves **conservatively** (start floors at ~10, not 5) with Flow Break already live.

---

## Part 1 — Server / admin tools (Skatha #1, #2)

### 1.1 Delete & rename players in-game

**Status:** no such commands exist (`interp.c` has no `delete`/`destroy`/`rename`/`pdelete`).
Today it's a server-side file operation.

**Proposal — two L_SUP/L_DEI commands:**

- `pdelete <name> <name>` — require the name typed **twice** to confirm (Diku convention for
  destructive ops). Steps: locate the char (online → force-disconnect/save-skip; offline →
  load enough to validate), delete the pfile at `player/<initial>/<Name>`, scrub from clan
  roster / leaders / notes authorship as needed, log to wiznet + the immortal log.
- `prename <old> <new>` — validate `new` with `check_parse_name`, reject if a pfile already
  exists for `new` (no clobber) or the player is currently online under `new`. Rename the
  pfile file, update `ch->name`, and re-save. If the target is online, do it on their loaded
  char and re-save; if offline, load → rename → save.

**Risks / care:**
- Destructive + irreversible → high trust level, double-confirm, always log.
- Name→file mapping is `player/<lowercased-initial>/<Name>`; rename must move between initial
  subdirs if the first letter changes.
- Watch for the target being mid-session (don't leave a stale descriptor pointing at a freed
  char; safest is "must be offline, or I'll disconnect them first").
- Clan leadership / hiscore tables keyed by name may dangle after delete/rename — decide
  whether to clean or leave.

### 1.2 New players start with 125 primal

**Status:** primal is **not** set at creation in `comm.c` (no `primal` assignment there). It's
initialized elsewhere (clear/char-init path) — **find the init site before coding** (likely the
nanny class-selection or a per-class setup in `comm.c`/`save.c`).

**Proposal:** grant **125 primal** to brand-new characters. Primal is the universal advancement
currency (Sorcerer research, Saiyan/Fist/etc. powers), so this helps *every* class get a couple
of starting abilities, not just Sorcerer.

**Balance check (Sorcerer specifically):** research cost is `min(75, rank*5)` per rank
(`sorcerer.c:332`), so ranks 1–5 of a school cost 5+10+15+20+25 = **75 primal**. 125 primal ≈
one school to rank ~6, or a couple of schools to rank 3–4 → exactly Skatha's "more options out
the gate than auto-attack." Reasonable. Make sure we don't *also* hand primal via training such
that 125 becomes 300+.

---

## Part 2 — Sorcerer bug fixes (verified)

| # | Bug | Where | Effect today |
|---|-----|-------|--------------|
| A | **Rank counted twice** — Mystic paid at start, then power recomputed from the *reduced* Mystic | `sorcerer.c` `do_chant` 129–155 vs `chant_cast` 173–174 (ignores `cast->rank`) | Expensive chants resolve *weaker* the more they cost |
| B | `holy_bless` strips the **caster** instead of the victim | `sorcerer.c:787-788` (`is_affected(victim…)` then `affect_strip(ch…)`) | Re-blessing eats the caster's own bless |
| C | `lighting` strips blindness from the **caster** before blinding the victim | `sorcerer.c:752-753` | Self-cures the caster's blindness for free; victim logic ok otherwise |
| D | `chant_laguna_blade` early-returns "out of order"; dead code below is itself broken | `sorcerer.c:722-740` (`do_remove(ch,"wield->name")` passes a literal string; `do_wear(ch,"23skidoo")`) | Spell does nothing |
| E | `chant_flow_break` is an empty `{}` | `sorcerer.c:805-807` | Spell does nothing |
| F | Recovery is handled in **two** update loops that contradict | `update.c:604-605` strips the bit; `update.c:1331-1353` heals-then-conditionally-strips | Murky/weak HoT |

### 2.1 The rank double-use bug — *the big one*

`do_chant` captures `rank = min(school, Mystic)` **before** payment and stores it in
`cha->rank` (`:155`). `chant_cast` then **ignores `cast->rank`** and recomputes
`rank = min(school, Mystic)` **after** Mystic was already reduced (`:173-174`).

Worked example (Skatha's, confirmed): school 50 / Mystic 50, rank-35 chant → captured 50, pay
35 → Mystic 15 → **resolves at 15.** The chant is ~3× weaker exactly because it was expensive.

**Fix:** in `chant_cast`, use `cast->rank` (delete the recompute). One line, but it's a *large*
top-end buff — see §3.1 for the balance framing and the refund idea.

### 2.2 Targeting fixes (B, C)

- `holy_bless`: change `affect_strip( ch, sn )` → `affect_strip( victim, sn )` so re-bless
  refreshes the *target*.
- `lighting`: the `if (is_affected(ch, blindness)) affect_strip(ch, blindness)` block operates
  on the caster — almost certainly meant the victim (refresh stacking blindness) or should be
  deleted. Recommend: **delete it** (don't auto-cure anyone); blindness already re-applies via
  `affect_to_char`. Worth a quick scan of the other white/elemental chants for the same
  copy-paste `ch`-vs-`victim` slip.

### 2.3 Laguna Blade / Flow Break stubs (D, E)

Not "tune these," they're **unfinished**. Designs in §3.4 / §3.5. Until implemented, leave
Laguna Blade returning its message (don't ship the broken dead code) and have Flow Break at
least print "not yet implemented" rather than silently succeeding.

### 2.4 Recovery split handling (F)

`update.c:604` removes `NEW_RECOVERY` in one loop while `update.c:1335+` heals from it in
another — so behavior depends on loop ordering and position. **Consolidate to one place** as
part of the §3.7 rework.

---

## Part 3 — Sorcerer balance

### 3.1 Stored rank + Mystic refund (Skatha balance #1)

Adopt: **pay Mystic at chant start, resolve at the captured rank** (the §2.1 fix).
Add: **refund 25–50% Mystic on miss/misfire** (`chant_cast` already has a misfire branch at
`:206`; many chants no-op on `saves_chant` — those should refund too).

Why this is the keystone: it makes power scale with *mastery* (your school rank, capped by
remaining Mystic) instead of *penalizing* you for casting your best spells. Sustained casting
still drains Mystic, so `min(school, Mystic)` naturally throttles you as you run low — that's
the intended pressure, and it stays intact.

> **VERIFY FIRST:** how fast does `SORC_MYSTIC` regenerate (grep `SORC_MYSTIC` in `update.c`)?
> If it regens fast, expensive chants are nearly free and the buff is bigger than it looks; if
> slow, the drain is a real resource and the change is well-contained. Tune refund % to match.

### 3.2 `saves_chant` rework (Skatha balance #2)

Current (`sorcerer.c:248-290`): `chance = 50` + victim defenses (+`level/3`, +`level+50` if
≥102 for NPCs; will/3 + class riders for PCs) − caster `will/3` − caster school rank;
if caster specialized, `chance /= 2`; then `URANGE(15, chance, 85)`.

So: hard **15% failure floor** even for a maxed specialist, and NPCs ≥102 rocket toward the
**85% ceiling**.

**Recommended numbers (slightly more conservative than Skatha's for PvP disables):**

| Lever | Now | Proposed |
|---|---|---|
| Base | 50 | **45** |
| Floor, specialized school | 15 | **8–10** (Skatha: 5) |
| Floor, off-spec | 15 | **12** (Skatha: 10) |
| Floor, PvE boss | 15 | **15** |
| Ceiling, normal | 85 | **75** |
| Ceiling, true anti-magic mob | 85 | **85** |

**Important nuance — split damage vs control.** A 5–8% floor is fine for *damage* chants, but
the same floor on **hard disables** (sleep, blindness, `laphas seed`/no-flee) means a maxed
sorc near-permalocks other players. Two options:
- give control chants a **higher floor** (12–15) than damage chants (8), via a per-chant flag; or
- keep the low floor but add **diminishing returns / brief immunity** after a disable lands in
  PvP (can't be re-slept for N ticks).

Either way, this tuning should ship **with Flow Break live** (§3.5) so there's an answer.

### 3.3 Specialization: rewarding, not punishing (Skatha balance #3)

Current (`sorcerer.c:326-421`): spec cap 50, off-spec cap 40 (+`rank` extra primal per rank),
respec costs **500 primal and hard-drops every school >40 back to 40** (`:417-420`) — i.e. it
*destroys* invested primal.

**Proposal:**
- Spec cap **50** (keep). Off-spec cap **42–45** (raise from 40). Shamanism elemental cap stays
  ~50 (already granted via the astral-spec branch `:329`).
- Respec **250–300 primal**, *or* one free respec per remort/rebirth.
- **Stop the hard-drop.** Don't delete over-40 ranks — **mark them inactive**: keep the raw
  number, but resolve chants at `effective = (school == spec || astral-elemental) ? raw :
  min(raw, offspec_cap)`. When you spec back, the full rank returns. This is the big QoL win —
  experimentation stops being punished.

**Implementation note:** "inactive, not dropped" means introducing an `effective_rank(ch,
school)` helper and using it **consistently** in `do_chant` (the `rank` gate `:55` and capture
`:129`), `chant_cast` (resolution — once §2.1 lands it reads `cast->rank`, so capture is the
only place), `saves_chant` (`:279`), and the research cap check. Single helper, several call
sites — easy to get subtly wrong, so centralize it.

### 3.4 Finish Laguna Blade — the scary finisher (Skatha balance #4)

The scaffolding exists (the `LAGUNA_BLADE` object, the `NEW_LAGUNABLADE` bit). Design:

- **School/req:** Black (or Black for caster, counters via Astral). High rank (≈40+).
- **Effect:** force-create + force-wield a temp blade for **1–2 ticks**; very high damage that
  **partly ignores protection/resistance** (e.g. 25–40% bypass).
- **Cost:** high Mystic up front, **plus a per-combat-round Mystic drain** while the blade is
  active (caps uptime; if Mystic bottoms out, blade poofs).
- **Lockout:** while `NEW_LAGUNABLADE` is set, **block other complex chants** (`lines > 1`) —
  the `do_chant` guard at `:67` already blocks two complex chants, so extend that condition.
- **Counterplay:** **disarm** (blade gone), **flee**, **Flow Break** (strips
  `NEW_LAGUNABLADE` → blade poofs), anti-magic.

**Implementation care (the original code's real failures):**
- Replace `do_remove(ch,"wield->name")` (passes a literal string!) with a real unwield of the
  current weapon, and `do_wear(ch,"23skidoo")` with a proper force-wield helper.
- Robust cleanup: the blade must vanish on expiry **and** on disarm/death/Flow Break, with no
  object leak and no leftover `NEW_LAGUNABLADE` bit. Centralize "end laguna" so every path uses
  it.

### 3.5 Finish Flow Break — the keystone counter (Skatha balance #4)

This is the **enabler** for the whole offensive side. Design (White/Astral):

- **Dispel 1–3 magical affects** scaled by rank (1 at low, 3 at high).
- **On failed save by the target, cancel one pending chant/prep** (their `pcdata->chant`).
- **Bonus vs:** magical barriers, elemental wards/blocks, **Laguna Blade** (poof it),
  **Recovery**.
- **PvP limit:** cannot fully strip everything in one cast (cap removals; pick highest-value or
  random subset) so it's a counter, not a one-button reset.

Ship this **before/with** the save-floor cut and Laguna Blade — without it, disables and the
finisher have no answer and PvP gets miserable.

### 3.6 Reliability before damage (Skatha balance #5)

Make early Sorcerer feel functional, then make the top end scary.

- **Rank 1–20 chants:** lower lag/wait; **partial damage on save** (½) instead of total whiff.
- **Rank 21–35:** normal saves, better utility riders.
- **Rank 36–50:** big effects, longer chant, **room-wide telegraph** ("the air screams...") so
  opponents can react (counterplay for the low save floors).

**Implementation:** cleanest is to let damage chants apply **half on save** rather than `return`
on `saves_chant` — either a shared helper returning a multiplier, or a per-chant "partial" flag.
Most damage chants currently `return` on save (e.g. `balus_rod` `:436`), so this is mechanical
but touches many functions; a helper keeps it uniform.

### 3.7 Make Recovery worth casting (Skatha balance #6)

Replace the bit-only design with a **defined heal-over-time**:

- **Immediate** small heal on cast.
- **2–3 pulse** heal-over-time (store a pulse counter, not just a bit, so it has a real length).
- **At higher rank:** also cleanse bleeding / poison / light stun.
- **No stack** (already enforced via the `NEW_RECOVERY` check at `sorcerer.c:841`).

**Implementation:** consolidate the two contradictory update branches (`update.c:604` and
`:1331-1353`) into one; back it with a small counter (e.g. a `powers[]` slot or an affect
duration) so the HoT length is explicit and removal is unambiguous.

---

## Part 4 — Sorcerer help & UX (Skatha #3)

All the data needed already lives in `chant_table` (`sorcerer.c:2166+`): each entry has
`school, prep, rank, cost, lines, lag, target` (+ a `wait`/trailing field) and `line1`. So these
are mostly *presentation* commands:

- **`chant list`** — list chants you can currently use (school owned at ≥ required rank), like
  the prep listing at `:2116`. Columns: name · school · rank req · Mystic cost.
- **`chant info <spell>`** — dump one chant: school, rank req, Mystic cost, lines (chant
  length), wait, lag, target type, prep-or-not, a short effect description, and **save behavior**
  (does it save? damage vs disable? partial-on-save?).
- **`research list`** — every school with your current rank, your cap (given spec), and the
  next rank's primal cost (straight from the `do_research` math).
- **Help text** (these are `help.are` data entries, not code): `help sorcerer`, `help chant`,
  `help research`, `help mystic` explaining: the 7 schools, **Mystic** as the spendable power
  pool (and how `min(school, Mystic)` sets your effective rank), specialization + caps,
  rank caps, what *lines/wait/lag/target* mean, and how **saves** work after the §3.2 rework.

These are safe, independent, and make every *other* change discoverable — good to land early so
players (and testers) can actually see the system.

---

## Balance interactions — read before coding

These stack. Doing them all in one drop risks Sorcerer going from underpowered to dominant:

- **Stored-rank fix (§3.1)** alone multiplies high-rank chant output (the 15→50 example ≈ 3×).
- **Save floor cut (§3.2)** multiplies *land rate*, especially for specialists.
- **Partial-on-save (§3.6)** removes the remaining whiff downside.

Three multipliers on the same class. Recommended path:
1. Ship **§3.1** (+ refund) and the target-fix bugs. Observe real fights.
2. Ship **Flow Break (§3.5)** and confirm anti-magic / Holy Resist counter it.
3. Then tune **saves (§3.2)** starting at the *conservative* end (floors ~10/12/15, ceiling 75).
4. Add **reliability (§3.6)** and **Laguna Blade (§3.4)** with counterplay already live.
5. Recovery, docs, admin whenever.

If we can route the key numbers (base save, floors, ceiling, respec cost, refund %, starting
primal) through a small config/const block, tuning stays a one-line change instead of a
recompile-and-hunt.

---

## Open questions / verify before implementation

1. **`SORC_MYSTIC` regen rate** — gates how meaningful the Mystic cost is (and thus how big §3.1
   really is). Grep `SORC_MYSTIC` in `update.c`.
2. **Where primal is initialized** for new chars (for the 125 grant) — not in `comm.c`.
3. **`LAGUNA_BLADE` object definition** (vnum/type/values) — confirm the prototype is sane for a
   1–2 tick weapon and that creating it won't re-trigger the old "bad type" class of issue.
4. **Per-chant save intent** — which chants are damage (want partial-on-save / low floor) vs
   hard control (want higher floor / DR) — needs a quick pass over the chant list with Skatha.
5. **PvP disable stacking** — decide on diminishing returns/immunity for sleep/blind/no-flee
   before cutting the floor.
6. **Delete/rename trust level + online-target policy** — must be offline, or auto-disconnect?

---

*Next step: pick the first slice (recommend §3.1 + the §2.2 target fixes — small, high-value,
and they let us feel the class before tuning). Say the word and I'll implement in the order
above, testing on the live server between balance changes.*
