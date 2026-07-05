import { useState } from 'react';
import {
  Box,
  Chip,
  Collapse,
  Divider,
  IconButton,
  LinearProgress,
  Paper,
  Stack,
  TextField,
  Typography,
  useMediaQuery,
  useTheme,
} from '@mui/material';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import ExpandLessIcon from '@mui/icons-material/ExpandLess';
import type { MudStatus } from '../lib/mud-connection';
import { GmcpMiniMap } from './gmcp-mini-map';

// GMCP side-panel for the /play terminal. Pure presentation: play.page owns the
// MudConnection, parses each GMCP module into the state below, and passes it in.
// Grays out / clears when the connection isn't live.

export interface Vitals {
  hp: number;
  maxhp: number;
  mana: number;
  maxmana: number;
  move: number;
  maxmove: number;
  res: number;
  maxres: number;
  resname: string;
}

export interface CharStatus {
  name?: string;
  class?: string;
  level?: number;
  exp?: number;
  gold?: number;
  primal?: number;
  area?: string; // not emitted by Char.Status on this server; we fill from Room.Info
  num?: number;
}

export interface RoomInfo {
  num: number;
  name: string;
  area: string;
  exits: Record<string, number>; // {"n":305,...}
}

export interface GmcpState {
  vitals: Vitals | null;
  status: CharStatus | null;
  room: RoomInfo | null;
  commands: string[]; // game.Commands — the player's available command names
  chants: string[]; // game.Chants — Sorcerer-only; empty for other classes
}

export interface GmcpPanelProps {
  connection: MudStatus;
  state: GmcpState;
  // Inject "<text>" into the terminal input and focus it (no auto-send).
  onInsertCommand?: (text: string) => void;
}

const DIRW: Record<string, string> = { n: 'N', e: 'E', s: 'S', w: 'W', u: 'Up', d: 'Down' };

function fmt(n: number | undefined): string {
  if (n == null || Number.isNaN(n)) return '—';
  return n.toLocaleString();
}

function Gauge({ label, value, max, color }: { label: string; value: number; max: number; color: string }) {
  const hasMax = max > 0 && !Number.isNaN(max);
  const pct = hasMax ? Math.max(0, Math.min(100, (value / max) * 100)) : 0;
  return (
    <Box>
      <Stack
        direction="row"
        sx={{
          justifyContent: "space-between",
          alignItems: "baseline",
          mb: 0.25
        }}>
        <Typography variant="caption" sx={{ color: 'text.secondary', textTransform: 'uppercase', letterSpacing: 0.5 }}>
          {label}
        </Typography>
        <Typography variant="caption" sx={{ fontFamily: 'monospace', color: 'text.primary' }}>
          {hasMax ? `${fmt(value)} / ${fmt(max)}` : '—'}
        </Typography>
      </Stack>
      <LinearProgress
        variant="determinate"
        value={pct}
        sx={{
          height: 8,
          borderRadius: 1,
          bgcolor: 'rgba(255,255,255,0.08)',
          '& .MuiLinearProgress-bar': { bgcolor: color, borderRadius: 1 },
        }}
      />
    </Box>
  );
}

function StatItem({ label, value }: { label: string; value: string }) {
  return (
    <Box sx={{ minWidth: 0 }}>
      <Typography variant="caption" sx={{ color: 'text.secondary', display: 'block', textTransform: 'uppercase', fontSize: 10, letterSpacing: 0.5 }}>
        {label}
      </Typography>
      <Typography variant="body2" noWrap sx={{ fontFamily: 'monospace' }} title={value}>
        {value}
      </Typography>
    </Box>
  );
}

function SectionTitle({ children }: { children: React.ReactNode }) {
  return (
    <Typography variant="subtitle2" sx={{ color: 'primary.main', mb: 1, fontWeight: 700 }}>
      {children}
    </Typography>
  );
}

// A row of clickable name chips. Clicking a chip hands `prefix + name + ' '` to
// the terminal input (e.g. "look " or "chant fireball ") so the user can add
// args and press Enter — it never auto-sends.
function ChipRow({ names, prefix, onInsert }: { names: string[]; prefix?: string; onInsert?: (text: string) => void }) {
  return (
    <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 0.5 }}>
      {names.map((n) => (
        <Chip
          key={n}
          size="small"
          variant="outlined"
          label={n}
          clickable
          onClick={() => onInsert?.(`${prefix ?? ''}${n} `)}
        />
      ))}
    </Box>
  );
}

// The command list can be 100+ entries, so this section is collapsible (default
// collapsed) and, when open, offers a type-to-narrow filter. When expanded the
// chip area flex-grows to absorb the panel's remaining vertical space (scrolling
// within it) so a tall screen isn't wasted; when collapsed it's just the header.
// We conditionally render the body rather than using <Collapse> because a
// Collapse wrapper has auto (content-driven) height, which would defeat the
// flex:1 fill — the toggle still shows/hides the body.
function CommandsSection({ commands, onInsert }: { commands: string[]; onInsert?: (text: string) => void }) {
  const [open, setOpen] = useState(false);
  const [filter, setFilter] = useState('');
  if (commands.length === 0) return null;
  const f = filter.trim().toLowerCase();
  const shown = f ? commands.filter((c) => c.toLowerCase().includes(f)) : commands;
  return (
    <Box sx={{ display: 'flex', flexDirection: 'column', minHeight: 0, flex: open ? 1 : '0 0 auto' }}>
      <Stack
        direction="row"
        sx={{
          alignItems: "center",
          justifyContent: "space-between"
        }}>
        <SectionTitle>Commands ({commands.length})</SectionTitle>
        <IconButton
          size="small"
          onClick={() => setOpen((o) => !o)}
          aria-label={open ? 'Collapse commands' : 'Expand commands'}
        >
          {open ? <ExpandLessIcon /> : <ExpandMoreIcon />}
        </IconButton>
      </Stack>
      {open && (
        <>
          <TextField
            size="small"
            fullWidth
            value={filter}
            onChange={(e) => setFilter(e.target.value)}
            placeholder="Filter commands…"
            autoComplete="off"
            sx={{ mb: 1 }}
          />
          {/* flex:1 fills the leftover panel height; minHeight keeps it usable on
              short screens; overflow scrolls within whatever height is left. */}
          <Box sx={{ flex: 1, minHeight: 96, overflowY: 'auto', pr: 0.5 }}>
            {shown.length === 0 ? (
              <Typography variant="body2" sx={{
                color: "text.secondary"
              }}>
                No matching commands.
              </Typography>
            ) : (
              <ChipRow names={shown} onInsert={onInsert} />
            )}
          </Box>
        </>
      )}
    </Box>
  );
}

export function GmcpPanel({ connection, state, onInsertCommand }: GmcpPanelProps) {
  const theme = useTheme();
  const isNarrow = useMediaQuery(theme.breakpoints.down('md'));
  const [open, setOpen] = useState(true);

  const live = connection === 'connected';
  const { vitals, status, room, commands, chants } = state;
  const area = room?.area ?? status?.area;

  const body = (
    <Stack
      spacing={2}
      sx={{
        // Fill the panel's height so the Commands chip list (flex:1) can grow
        // into the leftover space; the fixed sections keep their natural height.
        flex: 1,
        minHeight: 0,
        opacity: live ? 1 : 0.5,
        transition: 'opacity 0.2s',
        pointerEvents: live ? 'auto' : 'none',
      }}
    >
      {/* Vitals */}
      <Box>
        <SectionTitle>Vitals</SectionTitle>
        <Stack spacing={1.25}>
          <Gauge label="HP" value={vitals?.hp ?? 0} max={vitals?.maxhp ?? 0} color="#c0504d" />
          <Gauge label="Mana" value={vitals?.mana ?? 0} max={vitals?.maxmana ?? 0} color="#4b86d6" />
          <Gauge label="Move" value={vitals?.move ?? 0} max={vitals?.maxmove ?? 0} color="#7fae7f" />
          {vitals?.resname ? (
            <Gauge label={vitals.resname} value={vitals.res} max={vitals.maxres} color="#c9a24b" />
          ) : null}
        </Stack>
      </Box>

      <Divider />

      {/* Status */}
      <Box>
        <SectionTitle>Character</SectionTitle>
        {status ? (
          <Box
            sx={{
              display: 'grid',
              gridTemplateColumns: 'repeat(2, minmax(0, 1fr))',
              gap: 1,
            }}
          >
            <StatItem label="Name" value={status.name ?? '—'} />
            <StatItem label="Class" value={status.class ?? '—'} />
            <StatItem label="Level" value={fmt(status.level)} />
            <StatItem label="Exp" value={fmt(status.exp)} />
            <StatItem label="Gold" value={fmt(status.gold)} />
            <StatItem label="Primal" value={fmt(status.primal)} />
            {area ? <StatItem label="Area" value={area} /> : null}
          </Box>
        ) : (
          <Typography variant="body2" sx={{
            color: "text.secondary"
          }}>
            {live ? 'Waiting for character data…' : 'Not connected.'}
          </Typography>
        )}
      </Box>

      <Divider />

      {/* Room */}
      <Box>
        <SectionTitle>Room</SectionTitle>
        {room ? (
          <Stack spacing={1}>
            <Box>
              <Typography variant="body2" sx={{ fontWeight: 600 }}>
                {room.name || `#${room.num}`}
              </Typography>
              <Typography variant="caption" sx={{
                color: "text.secondary"
              }}>
                #{room.num}
                {room.area ? ` · ${room.area}` : ''}
              </Typography>
            </Box>
            <Box>
              <Typography
                variant="caption"
                sx={{
                  color: "text.secondary",
                  display: 'block',
                  mb: 0.5
                }}>
                Exits
              </Typography>
              {Object.keys(room.exits).length === 0 ? (
                <Typography variant="body2" sx={{
                  color: "text.secondary"
                }}>
                  none
                </Typography>
              ) : (
                <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 0.5 }}>
                  {Object.keys(room.exits).map((dir) => (
                    <Chip key={dir} size="small" variant="outlined" label={DIRW[dir.toLowerCase()] ?? dir} />
                  ))}
                </Box>
              )}
            </Box>
            <GmcpMiniMap num={room.num} />
          </Stack>
        ) : (
          <Typography variant="body2" sx={{
            color: "text.secondary"
          }}>
            {live ? 'Waiting for room data…' : 'Not connected.'}
          </Typography>
        )}
      </Box>

      {/* Commands — clickable helpers; long, so collapsible + filterable. */}
      {commands.length > 0 && (
        <>
          <Divider />
          <CommandsSection commands={commands} onInsert={onInsertCommand} />
        </>
      )}

      {/* Chants — Sorcerer-only; rendered only when the list is non-empty. */}
      {chants.length > 0 && (
        <>
          <Divider />
          <Box>
            <SectionTitle>Chants</SectionTitle>
            <ChipRow names={chants} prefix="chant " onInsert={onInsertCommand} />
          </Box>
        </>
      )}
    </Stack>
  );

  return (
    <Paper
      variant="outlined"
      sx={{
        bgcolor: '#121217',
        borderColor: 'divider',
        p: 1.5,
        height: '100%',
        display: 'flex',
        flexDirection: 'column',
        minHeight: 0,
        overflowY: 'auto',
      }}
    >
      <Stack
        direction="row"
        sx={{
          alignItems: "center",
          justifyContent: "space-between",
          mb: isNarrow ? 0 : 1.5
        }}>
        <Typography variant="subtitle1" sx={{ fontWeight: 700 }}>
          Session
        </Typography>
        {isNarrow && (
          <IconButton size="small" onClick={() => setOpen((o) => !o)} aria-label={open ? 'Collapse panel' : 'Expand panel'}>
            {open ? <ExpandLessIcon /> : <ExpandMoreIcon />}
          </IconButton>
        )}
      </Stack>
      {isNarrow ? (
        <Collapse in={open}>
          <Box sx={{ pt: 1.5 }}>{body}</Box>
        </Collapse>
      ) : (
        body
      )}
    </Paper>
  );
}
