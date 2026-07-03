---
issue: 6
type: balance
status: in-progress
opened: 2026-06-27
reporter: ScathachMDG
epic: 1
---

# White Sorcerer support buffs (#6)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/6 · Epic: #1

Five small White-spec spell improvements (`src/classes/sorcerer.c`) to make White a real support
spec without making it a 1v1 powerhouse. Display/peel/utility focused; no damage buffs.

## Implemented
1. **Holy Bless** — better early scaling: modifier `UMAX(1,(rank-30)/2)` → **`UMAX(2, rank/5)`**
   (10→+2, 25→+5, 50→+10) on both hitroll and damroll.
2. **Dicleary** — healing/move unchanged; now also **cleanses one major debuff** by rank,
   highest-priority first: curse (35+, strips an elemental curse) > root/no-flee (45+, strips the
   affect carrying `AFF_NO_FLEE`) > blind (25+) > poison (15+). At most one per cast.
3. **Chaos Strings** — now **applies `AFF_CHAOS_STRING`** (the fight code already halves Fist
   melee and cuts attacks ~⅓ for it; the chant never applied it before). Save-scaled duration
   (fail 2t PvP / 4t PvE; save 1t / 2t). Damage unchanged — value is the peel.
4. **Lighting** — no longer all-or-nothing: on a save, a brief **Dazzled** hitroll −3 (1 tick);
   on a failed save, **blind** with hitroll −8, PvP duration capped at 2 ticks.
5. **Dark Mist** — was a stub; now does something: a **−10 hitroll** dimming on all hostiles in
   the room at cast (short duration).

## Acceptance criteria
- [ ] Holy Bless gives meaningful low-rank hit/dam; scales to +10 at 50.
- [ ] Dicleary still heals AND removes one debuff appropriate to rank.
- [ ] Chaos Strings leaves a target whose melee output is visibly reduced (esp. Fist).
- [ ] Lighting on a save → short hitroll penalty (not nothing); on fail → capped blind.
- [ ] Dark Mist applies −10 hitroll to room enemies and announces itself.

## Deferred (documented, not done this pass)
- **Holy Bless white-specialist resist bonus** (+5% vs curse/blind/sleep/no-flee): cross-cutting
  hook into every effect-application site; deferred to avoid a risky broad change.
- **Holy Bless PvP ally-target restriction** (grouped-only): targeting is enforced at the cast
  layer, not the chant; left as-is for now.
- **Dark Mist full design**: persistent room field, ally +15% flee, scan/ranged penalties — the
  first pass above is the reporter's reduced scope.

## Risk & balance notes
- All numbers are the reporter's proposal; tune in playtest. No damage buffs, so low power-creep risk.
