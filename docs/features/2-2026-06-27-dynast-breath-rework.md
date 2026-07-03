---
issue: 2
type: balance
status: spec-review
opened: 2026-06-27
reporter: ScathachMDG
epic: 1
---

# Dynast Breath rework — single clear effect (#2)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/2
Epic: #1 · **Reporter's #1 priority** ("Dynast Breath is making Sorcerer too powerful — tone it way down without making it useless").

## Summary
Dynast Breath is currently a no-save, long-duration stack of four debuffs that lets a
Sorcerer win by default. Collapse it to **one clear effect — no-flee + AC exposure —
add a save, and drastically shorten the duration**, keeping it strong-when-it-lands but
no longer an automatic lockdown.

## Reporter's request
Tone Dynast Breath down per the patch in #1 without making it useless: remove the
hitroll/damroll suppression, add a save, cap/lower the AC penalty, shorten duration,
and make removal clean. Numbers are the target feel; final values get a balance pass.

## Current behavior (verified — `src/classes/sorcerer.c:912 chant_dynast_breath`)
- **No save** — applies unconditionally.
- Three affects, all under the `dynast breath` skill type:
  1. `APPLY_AC`, modifier `rank * 8` (rank 50 → **+400 AC**), bitvector `AFF_NO_FLEE`.
  2. `APPLY_HITROLL`, modifier `victim->hitroll * -2 * rank / 110` (snapshot at cast).
  3. `APPLY_DAMROLL`, modifier `victim->damroll * -2 * rank / 110` (snapshot at cast).
- **Duration `dice(10,20)` ≈ 105 ticks (~50 min)** for all of the above.
- Note: hit/damroll are snapshotted from the victim's current values and scale oddly on
  NPCs (mob stat modifiers aren't applied the same way as PCs).

## Proposed design
Reduce to **one effect: no-flee (on failed save) + a capped AC penalty**. Add a save.
Remove the hitroll and damroll affects entirely.

- Call `saves_chant( ch, victim, cn )` (mirrors other saved chants in this file).
- Apply a **single** `dynast breath` affect: `APPLY_AC` with the capped modifier below,
  `AFF_NO_FLEE` only on a failed save. Because affects in this codebase are static
  modifiers, set the AC value **at cast time** by target type — `pvp = !IS_NPC(victim)` —
  rather than a per-round combat hook (simpler, matches how the chant already uses
  `APPLY_AC`). Remove the separate `APPLY_HITROLL` / `APPLY_DAMROLL` affects.
- Since it's one affect of one type, a single dispel / Flow Break / `affect_strip`
  removes the whole effect (already true by type — keep it to one affect so it stays clean).

Target numbers (PvP = victim is a PC) — **subject to balance pass before build:**

| | Duration | No-flee | AC penalty (rank 50) |
|---|---|---|---|
| PvP, failed save | 3 ticks | yes | `UMIN(rank*4, 200)` → 200 |
| PvP, successful save | 1 tick | no | `UMIN(rank*2, 100)` → 100 |
| PvE, failed save | 5 ticks | yes | `rank*6` → 300 |
| PvE, successful save | 2 ticks | no | `rank*3` → 150 |

Net effect vs today: adds a save, removes hit/damroll, **AC penalty 400 → ≤200 in PvP**,
and **duration ~105 ticks → 1–5 ticks** (the dominant change).

## Acceptance criteria
- [ ] Dynast Breath calls `saves_chant`; success and failure produce the durations/AC above.
- [ ] No `APPLY_HITROLL` or `APPLY_DAMROLL` affect is applied (verify via `affects` on a hit target).
- [ ] Exactly one `dynast breath` affect on the target; one dispel/Flow Break clears it entirely.
- [ ] `AFF_NO_FLEE` present only on a failed save; absent on a save.
- [ ] AC penalty matches PvP vs PvE target type and respects the caps.
- [ ] Dev verification: cast on a PC dummy and an NPC; confirm `affects`/`score` AC delta,
      no-flee behavior (try to flee), and that the effect expires on schedule.

## Risk & balance notes
- **Numbers need your balance pass** (reporter agreed). The duration cut is the biggest lever
  — confirm 3/1 PvP and 5/2 PvE feel right, or adjust.
- Keep "strong when prepared": this should still meaningfully expose a target, just not auto-win.
- Low blast radius: change is contained to `chant_dynast_breath`; no shared helpers altered.
- Watch interaction with Flow Break (#8) and dispel — should remain a single clean removal.

## Out of scope
- Other Sorcerer chants (Blast Ash cap is #5; Flow Break is #8).
- Any new AFF bits — this reuses `AFF_NO_FLEE` and `APPLY_AC`.
