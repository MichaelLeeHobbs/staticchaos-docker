# Static Chaos — Mudlet client packages

Client-side add-ons that use the server's **GMCP** (telnet option 201) data.

## StaticChaos-Gauges.xml — status bars

Draws **HP / Mana / Move** gauges across the bottom of the Mudlet window and
updates them in real time from the server's `Char.Vitals` GMCP messages.

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
- The gauges are positioned at the very bottom; if they overlap your input line,
  edit the `y="-2.4c"` values in the script (more-negative = higher up).
- No commands or settings to configure — it's purely driven by `Char.Vitals`.

## Coming next
- **Auto-mapper** — the server now also sends `Room.Info {num,name,area,exits}`
  on each room change (Phase 2). A small mapper script (or Mudlet's built-in
  mapper hooked to GMCP) will turn that into a live map. *(Phase 2 is staged on
  the server but not yet deployed.)*
- **Tab completion** — once the server emits command/chant lists (Phase 3), a
  script here can do context-aware Tab completion.
