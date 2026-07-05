import { useCallback, useRef, useState } from 'react';
import { Box, Typography, Alert } from '@mui/material';
import { MudTerminal, type MudTerminalHandle } from '../components/mud-terminal';
import { GmcpPanel, type GmcpState, type Vitals, type CharStatus, type RoomInfo } from '../components/gmcp-panel';
import type { GmcpMessage, MudStatus } from '../lib/mud-connection';

const EMPTY: GmcpState = { vitals: null, status: null, room: null, commands: [], chants: [] };

// Coerce a GMCP field to a number (the proxy/latin1 path can hand values back as
// strings), tolerating missing keys.
function num(v: unknown): number {
  const n = Number(v);
  return Number.isNaN(n) ? 0 : n;
}

// game.Commands / game.Chants arrive as JSON arrays of name strings.
function strArray(v: unknown): string[] {
  return Array.isArray(v) ? v.filter((x): x is string => typeof x === 'string') : [];
}

export function PlayPage() {
  const [status, setStatus] = useState<MudStatus>('disconnected');
  const [gmcp, setGmcp] = useState<GmcpState>(EMPTY);
  const termRef = useRef<MudTerminalHandle>(null);

  // Command/chant chips inject "<command> " into the input and focus it (no
  // auto-send — many commands take args). The terminal owns the input line.
  const insertCommand = useCallback((text: string) => {
    termRef.current?.insertInput(text);
  }, []);

  const handleStatus = useCallback((s: MudStatus) => {
    setStatus(s);
    // Fresh session on a real drop; keep last-known during a 'connecting' blip.
    if (s === 'disconnected' || s === 'error') setGmcp(EMPTY);
  }, []);

  const handleGmcp = useCallback((msg: GmcpMessage) => {
    const d = (msg.data ?? {}) as Record<string, unknown>;
    switch (msg.module) {
      case 'Char.Vitals':
        setGmcp((g) => ({
          ...g,
          vitals: {
            hp: num(d.hp),
            maxhp: num(d.maxhp),
            mana: num(d.mana),
            maxmana: num(d.maxmana),
            move: num(d.move),
            maxmove: num(d.maxmove),
            res: num(d.res),
            maxres: num(d.maxres),
            resname: typeof d.resname === 'string' ? d.resname : '',
          } satisfies Vitals,
        }));
        break;
      case 'Char.Status':
        setGmcp((g) => ({
          ...g,
          status: {
            name: d.name != null ? String(d.name) : undefined,
            class: d.class != null ? String(d.class) : undefined,
            level: d.level != null ? num(d.level) : undefined,
            exp: d.exp != null ? num(d.exp) : undefined,
            gold: d.gold != null ? num(d.gold) : undefined,
            primal: d.primal != null ? num(d.primal) : undefined,
            area: d.area != null ? String(d.area) : undefined,
            num: d.num != null ? num(d.num) : undefined,
          } satisfies CharStatus,
        }));
        break;
      case 'Room.Info': {
        const exits: Record<string, number> = {};
        const rawExits = d.exits;
        if (rawExits && typeof rawExits === 'object') {
          for (const [dir, to] of Object.entries(rawExits as Record<string, unknown>)) exits[dir] = num(to);
        }
        setGmcp((g) => ({
          ...g,
          room: {
            num: num(d.num),
            name: d.name != null ? String(d.name) : '',
            area: d.area != null ? String(d.area) : '',
            exits,
          } satisfies RoomInfo,
        }));
        break;
      }
      // Static, once-per-login lists. Retained across a 'connecting' blip (only
      // wiped on a real disconnect/error, via handleStatus). game.Chants is sent
      // only for Sorcerers; the panel hides its section when the list is empty.
      case 'game.Commands':
        setGmcp((g) => ({ ...g, commands: strArray(msg.data) }));
        break;
      case 'game.Chants':
        setGmcp((g) => ({ ...g, chants: strArray(msg.data) }));
        break;
      // Core.Hello / Client.GUI and any other modules are ignored.
      default:
        break;
    }
  }, []);

  return (
    <Box sx={{ display: 'flex', flexDirection: 'column', flex: 1, minHeight: 0 }}>
      <Typography variant="h1" gutterBottom>
        Play Now
      </Typography>
      <Typography
        variant="body1"
        sx={{
          color: "text.secondary",
          mb: 2,
          maxWidth: 760
        }}>
        Play Static Chaos right here in your browser &mdash; no telnet client, no install. Hit
        <strong> Connect</strong>, then type commands in the input line below the screen. The side panel
        shows your vitals, character and a live mini-map via GMCP.
      </Typography>
      <Alert severity="info" sx={{ mb: 2, maxWidth: 760 }}>
        New here? Type <code>new</code> at the name prompt to create a character. Your password is
        hidden while typing at password prompts.
      </Alert>
      <Box
        sx={{
          flex: 1,
          minHeight: 0,
          display: 'flex',
          flexDirection: { xs: 'column', md: 'row' },
          gap: 1.5,
        }}
      >
        {/* Terminal fills the main area; its own ResizeObserver refits xterm when
            this box resizes (column⇄row switch, panel collapse). */}
        <Box sx={{ flex: 1, minWidth: 0, minHeight: { xs: 360, md: 0 } }}>
          <MudTerminal ref={termRef} onGmcp={handleGmcp} onStatus={handleStatus} />
        </Box>
        {/* Fixed-width panel on desktop; full-width (collapsible) on mobile. */}
        <Box sx={{ width: { xs: '100%', md: 330 }, flexShrink: 0, minHeight: 0 }}>
          <GmcpPanel connection={status} state={gmcp} onInsertCommand={insertCommand} />
        </Box>
      </Box>
    </Box>
  );
}
