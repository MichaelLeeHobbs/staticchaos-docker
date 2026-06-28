---
issue: 34
type: balance
status: spec-review
opened: 2026-06-28
reporter: ScathachMDG
---

# Dynast Breath — restore not-saved duration (#34)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/34

## Summary
The #2 rework cut Dynast Breath's duration too hard. Restore the **not-saved** version to a long
range, keep the save shortening it. Playtest follow-up to #2.

## Reporter's decision (Skatha) — exact code
```c
if ( saved )
    af.duration = !IS_NPC(victim) ? dice(2, 10) : dice(3, 10);
else
    af.duration = dice(10, 20);
```

## Current state (verified — sorcerer.c:978)
`af.duration = saved ? ( pvp ? 1 : 2 ) : ( pvp ? 3 : 5 );`
The #2 rework deliberately replaced the old **~105-tick lockdown** with this short window (its
comment: "Drops … the ~105-tick lockdown that let Sorcerer win by default"), and added the save +
the Holy Bless ward (#23) + the AC-exposure model.

## Proposed change
Replace only the duration line (978) with Skatha's conditional. Everything else stays: AC penalty
(`ac_pen`), `AFF_NO_FLEE` only on a failed save, the Holy Bless ward shrug (#23), the "already
encased" guard.

## ⚠️ Decisions for review (maintainer) — this is a real reversal
- **Magnitude:** `dice(10, 20)` = **10–200 ticks (avg ~105)** — i.e. it restores the very lockdown
  #2 removed for being "win by default." It's now gated behind a failed save + the Holy Bless ward,
  which is the counterplay that didn't exist before — so it's *defensible*, but confirm we want the
  long root back.
- **No PvP cap:** her not-saved `dice(10,20)` is flat — current code caps PvP shorter than PvE
  (3 vs 5). Her version drops that split, so a **PvP** target who fails the save eats a 10–200 tick
  no-flee root. Confirm that's intended, or keep a PvP cap (e.g. not-saved PvP = `dice(5,10)`).
- Her **saved** durations also rise (PvP `dice(2,10)`=2–20, PvE `dice(3,10)`=3–30) vs current 1/2.

## Acceptance criteria
- [ ] Not-saved: duration `dice(10,20)`; saved: `dice(2,10)` (PvP) / `dice(3,10)` (PvE) — or the
      maintainer-chosen PvP-capped variant.
- [ ] AC penalty, save gating, Holy Bless ward, and "already encased" guard all unchanged.

## Risk & balance notes
- Large swing (current not-saved 3–5 → 10–200). Reverses the #2 anti-lockdown intent; only the save
  + ward keep it fair. Strongly playtest in PvP.
