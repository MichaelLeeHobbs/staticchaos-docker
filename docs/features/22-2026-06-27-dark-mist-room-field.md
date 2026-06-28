---
issue: 22
type: balance
status: spec-review
opened: 2026-06-27
reporter: ScathachMDG
epic: 1 (follow-up)
---

# Dark Mist — full room-control field (#22)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/22

## Summary
Promote Dark Mist from its shipped **first pass** (a one-shot −10 hitroll on hostiles present at
cast, #6) to a **persistent room field**: lasts a few ticks, dims enemies, helps allies flee, and is
cleared by fire. This is the heaviest follow-up because the engine has **no room-affect system** —
it needs a small new mechanism.

## Key finding (verified)
- `ROOM_INDEX_DATA` (merc.h) has **no affect list** — only `people`, `room_flags`, `light`,
  `sector_type`, exits, etc. There is **no `affect_to_room`**, no room affect bits, and no precedent
  for a timed room effect. So a "room field" must be built, not reused.
- Hook points that *do* exist: `do_flee` (fight.c:3150), `do_scan` (act_info.c:2871), the to-hit
  calc and fire-damage path in fight.c.
- There is **no per-room update loop** (Merc doesn't tick every room) — so expiry needs either a
  small global list of active mists ticked from `update_handler`, or lazy expiry on access.

## Proposed design
A lightweight **active-mist registry** rather than per-room affect storage:

1. **State:** a small global list/array of `{ ROOM_INDEX_DATA *room; int timer; CHAR_DATA *owner; }`
   (only a handful active at once). Cast adds/refreshes an entry for `ch->in_room` with the caster as
   `owner` and a PvP-2t / PvE-5t timer.
2. **Expiry:** decrement timers in `update_handler` (alongside the existing `PULSE_*` blocks); on 0,
   remove + emit the fade message to the room.
3. **Effects while active** (look up the caster's room in the registry):
   - **Enemies** (not grouped with `owner`): −10 hitroll — applied in the to-hit calc in fight.c.
   - **Allies** (grouped with `owner`): +15% flee chance — in `do_flee`.
   - *(Optional, phase 2)* enemy scan/vision and ranged accuracy reduced — `do_scan` + ranged path.
4. **Fire clears it:** in the fire-damage path (and Patryn fire room effects), remove the room's
   registry entry + "The dark mist burns away." message.
5. **Messages:** create ("A dark mist spills across the room…"), fade, fire-clear.

### Decisions for review (maintainer)
- **Owner-pointer safety (must-resolve):** storing `CHAR_DATA *owner` risks a dangling pointer if
  the caster is extracted (quit/death). Options: (a) null the entry's owner in `extract_char`
  (clean, one hook); (b) store the owner's group leader / a side token instead; (c) drop the
  ally/enemy split and make the mist symmetric (−10 hitroll to everyone, no ally-flee) — simplest,
  but loses the support flavor. **Recommend (a).**
- **Scope:** full (hitroll + ally-flee + scan/ranged + fire-clear) vs a **phase-1** (hitroll +
  ally-flee + fire-clear, defer scan/ranged) — the reporter explicitly allowed the reduced first
  pass. **Recommend phase-1 first.**
- **Worth it?** A working first-pass already ships; this is polish that adds real engine surface.
  Fine to greenlight, or to keep the first-pass and close #22 as "good enough."

## Acceptance criteria (phase-1)
- [ ] Casting Dark Mist marks the room for PvP 2 / PvE 5 ticks; re-cast refreshes.
- [ ] While active: non-allies in the room have −10 hitroll; grouped allies get +15% flee.
- [ ] People who **enter** after the cast are affected (it's a field, not a one-shot).
- [ ] Fire damage in the room clears the mist with a message; it also expires on its timer.
- [ ] Owner leaving/dying does not crash (owner-safety decision implemented).
- [ ] Dev verification: cast, have a non-ally enter and check hitroll; flee as an ally; burn it with fire.

## Risk & balance notes
- New subsystem touching update loop + to-hit + flee (+ scan/fire). Highest-surface follow-up;
  owner-pointer safety is the main correctness risk.
- Numbers (2/5 ticks, −10, +15%) are the reporter's proposal; tune in playtest.
- The shipped first-pass (`chant_dark_mist` per-char debuff) is replaced by the registry version.

## Out of scope
- Generalizing into a full room-affect system for other spells (this builds only what Dark Mist needs).
