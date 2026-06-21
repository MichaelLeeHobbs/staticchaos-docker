# Open Items / TODO

Living list of open work, decisions, and tuning. All committed code is shipped & deployed; the items
below are mostly playtest tuning, verification, and optional follow-ups. (Detailed per-number tuning
notes are kept in the maintainers' private working notes.)

## Awaiting playtest / tuning (live, numbers are tunable)
- **Patryn feedback fixes (just shipped, tune from play):** earth-arm runes now buff **parry** (not
  damage; only fire arm adds damage); raised defenses also block **Astral Detect** for items the Patryn
  carries/wears (not `scan`); water-leg runes give a small **move-regen** buff (+2%/rune) on top of dodge.
- **Patryn patch** — these are "fine until testing": Spirit Ward soak % + drain, curse/dispel resist %,
  `defenses raised` buff/penalty numbers, the four elemental-curse numbers, water-torso-vs-cerulean.
- **Class Trials Phase B** — verify the XP + Primal reward actually fires on a first tier clear (could
  not be driven by a stat-less test immortal). Tune the per-tier required abilities and the POWERUP
  ⅓-charge threshold.
- **Sorcerer / Saiyan PvP balance** — shipped as proposed numbers (Ki Wave chance, Defense soak/absorb,
  Holy Resist & Vas Gluudo reductions, Scorched); awaiting play-test feedback.
- **Assessment fix** — confirm endgame characters previously stuck at "Unholy" now reach their class
  title (e.g. Rune Lord) on their next login.

## Optional / on request
- **NPC dialogue tuning** — report the vnum of any NPC that says something off-character → add a
  per-mob override or fix the role rule.
- **`abri.are`** — cannot be loaded until `labyrinth.are`'s broken exits (3,600+ Fix_exits) are
  repaired.
- **Trial-death corpse location** — currently routed to the Trial Lobby; can move to the temple (zero
  friction) or leave in-room (hardcore).
- **Class Trials "Phase C"** — true per-ability detection to replace the persistent-state proxy on the
  interrupt / school-switch / runeweave tiers (the gap between what shipped and the full design).
- **Sorcerer/Saiyan deferred sub-parts** — Holy Resist blocking one drain/curse/sleep/blind; Defense
  absorbing Dim-Mak / Mazoku release-blast interrupts (only Ki Wave is wired so far).

## Deferred decisions
- **Sorcerer "spells that don't work"** — the two-word/quote practice+cast parsing bug is fixed and no
  broken spell functions were found; needs specific spell names from a play-test to chase further.

## Recently shipped (reference)
Class Trials Phase B (per-class objectives) · trial polish (reset-HP on entry, win-or-die confirm,
corpse→lobby) · Hamster of Doom two-feet crafting (new `carries` mobprog ifcheck) · NPC keyword
dialogue (v2, rotation + role/chatter) · assessment/"level" cap fix · Sorcerer `mana_gain` sleeping
fall-through fix · Patryn patch Phases 1–3 · eval-class bug-hunt (no other bugs found).
