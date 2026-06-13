# Static Chaos — Mudlet client packages

Client-side add-ons that use the server's **GMCP** (telnet option 201) data.

## StaticChaos-Gauges.xml — status bars

Draws **HP / Mana / Move** gauges across the **top** of the Mudlet window (just
under the toolbar) and updates them in real time from the server's `Char.Vitals`
GMCP messages. It reserves a top border so the bars sit *above* the game text
rather than covering it.

### Install
1. In Mudlet: **Toolbox → Package Manager** (or **Settings → Packages**).
2. Click **Install**, choose `StaticChaos-Gauges.xml`.
3. Connect to the MUD — the three bars appear at the bottom and start moving.

(Alternatively: **Scripts** editor → the import/install button → pick the XML.)

### Requirements
- Mudlet (GMCP is on by default — no settings needed).
- The server must have GMCP enabled (Phase 0+1 — **live**). The server offers
  `IAC WILL GMCP` on connect and Mudlet replies automatically.

### Notes
- The bars sit at the top in a reserved 36px strip (`setBorderTop`). To make them
  taller/shorter, change `setBorderTop(36)` and the gauges' `height="32px"`
  together so they stay in sync.
- No commands or settings to configure — it's purely driven by `Char.Vitals`.

## StaticChaos-Mapper.xml — auto-mapper

Feeds Mudlet's **built-in mapper** from the server's `Room.Info` GMCP messages
(Phase 2, live). As you walk, each new room is drawn next to the one you came
from, with its exits.

### Install
Same as the gauges: **Toolbox → Package Manager → Install →** pick
`StaticChaos-Mapper.xml`. Then open the map with the **Mapper** toolbar button
(or Toolbox → Mapper) and start walking.

### How it works / notes
- Rooms are keyed by their MUD vnum, grouped into areas by name, and positioned
  by walking the exits (north = up on the map, etc.). Exits to rooms you haven't
  visited yet show as stubs and connect once you go there.
- The map is built as you explore — it won't show areas you haven't walked.
- The MUD isn't perfectly grid-shaped (some exits loop or stack), so a few rooms
  may overlap; that's normal for an organically-built map.
- **First run:** test it in Mudlet and watch the map populate as you move. If
  exits don't draw, tell me your Mudlet version — the exit-direction format is
  the one spot that occasionally differs between versions.

## Coming next
- **Tab completion** — once the server emits command/chant lists (Phase 3), a
  script here can do context-aware Tab completion.
- **One-click / auto-install** — later we can host these packages and have the
  server auto-deliver them via Mudlet's `Client.GUI` GMCP message (you'll drive
  the web-hosting side).
