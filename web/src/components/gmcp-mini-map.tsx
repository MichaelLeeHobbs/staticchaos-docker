import { useEffect, useMemo, useState } from 'react';
import { Box, Link, Typography } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import { loadWorld } from '../lib/data';
import type { World } from '../lib/types';

// Compact "you are here" mini-map for the GMCP side-panel.
//
// Reuses the World/Room shape that map.page loads (loadWorld → world.json;
// rooms keyed by vnum, each with exits: {dir: destVnum}). Rather than embedding
// the heavy d3 force-graph (map-graph.tsx) in a ~300px column — which would
// fight the layout and be illegible at that size — we draw a small compass SVG:
// the current room in the centre, its immediate neighbours placed by exit
// direction. If the current vnum isn't in the world data we render nothing and
// let the Room text panel carry the info (graceful degradation).

const DIRW: Record<string, string> = { n: 'north', e: 'east', s: 'south', w: 'west', u: 'up', d: 'down' };

// viewBox geometry — centre + a slot per compass direction.
const VB_W = 260;
const VB_H = 190;
const CX = VB_W / 2;
const CY = VB_H / 2;
const SLOT: Record<string, [number, number]> = {
  n: [CX, 30],
  s: [CX, VB_H - 30],
  e: [VB_W - 34, CY],
  w: [34, CY],
  u: [VB_W - 40, 40],
  d: [40, VB_H - 40],
};

const CENTER_FILL = '#c9a24b'; // primary gold — "you are here"
const NEIGHBOR_FILL = '#3a5a86';
const NEIGHBOR_STROKE = '#6f9ad6';
const EDGE = '#4a4a55';

const trunc = (s: string, n = 12) => (s.length > n ? `${s.slice(0, n - 1)}…` : s);

let worldCache: World | null = null;

export interface MiniMapProps {
  num: number | null; // current room vnum from Room.Info
}

export function GmcpMiniMap({ num }: MiniMapProps) {
  const navigate = useNavigate();
  const [world, setWorld] = useState<World | null>(worldCache);

  useEffect(() => {
    if (worldCache) return;
    let live = true;
    loadWorld()
      .then((w) => {
        worldCache = w;
        if (live) setWorld(w);
      })
      .catch(() => {
        /* the mini-map is optional; the Room text panel still renders */
      });
    return () => {
      live = false;
    };
  }, []);

  const view = useMemo(() => {
    if (!world || num == null) return null;
    const here = world.rooms[String(num)];
    if (!here) return null; // unknown vnum → degrade (render nothing)
    const neighbors = Object.entries(here.exits)
      .map(([dir, to]) => {
        const d = dir.toLowerCase();
        if (!SLOT[d]) return null;
        const dest = world.rooms[String(to)];
        return { dir: d, to, name: dest?.name ?? `#${to}`, crossArea: dest ? dest.area !== here.area : false };
      })
      .filter((n): n is NonNullable<typeof n> => n !== null);
    return { here, neighbors };
  }, [world, num]);

  if (!view) return null;

  return (
    <Box>
      <Box
        component="svg"
        viewBox={`0 0 ${VB_W} ${VB_H}`}
        sx={{ width: '100%', height: 'auto', display: 'block', userSelect: 'none' }}
        role="img"
        aria-label="Local area mini-map"
      >
        {/* edges first, so nodes draw on top */}
        {view.neighbors.map((n) => {
          const [x, y] = SLOT[n.dir]!;
          return <line key={`e-${n.dir}`} x1={CX} y1={CY} x2={x} y2={y} stroke={EDGE} strokeWidth={2} />;
        })}

        {/* neighbour nodes */}
        {view.neighbors.map((n) => {
          const [x, y] = SLOT[n.dir]!;
          return (
            <g
              key={`n-${n.dir}`}
              style={{ cursor: 'pointer' }}
              onClick={() => navigate(`/map?room=${n.to}`)}
            >
              <title>{`${DIRW[n.dir]}: ${n.name}${n.crossArea ? ' (other area)' : ''}`}</title>
              <circle
                cx={x}
                cy={y}
                r={13}
                fill={n.crossArea ? '#3a5f4a' : NEIGHBOR_FILL}
                stroke={n.crossArea ? '#5aa17a' : NEIGHBOR_STROKE}
                strokeWidth={1.5}
              />
              <text x={x} y={y + 4} textAnchor="middle" fontSize={11} fontWeight={700} fill="#e6e6ec">
                {n.dir.toUpperCase()}
              </text>
              <text x={x} y={y + 26} textAnchor="middle" fontSize={9} fill="#9a9aa4">
                {trunc(n.name, 14)}
              </text>
            </g>
          );
        })}

        {/* centre (current room) */}
        <g style={{ cursor: 'pointer' }} onClick={() => navigate(`/map?room=${view.here.vnum}`)}>
          <title>{`${view.here.name} (#${view.here.vnum}) — open full map`}</title>
          <circle cx={CX} cy={CY} r={17} fill={CENTER_FILL} stroke="#f0d9a0" strokeWidth={2} />
          <circle cx={CX} cy={CY} r={22} fill="none" stroke={CENTER_FILL} strokeWidth={1} opacity={0.4} />
        </g>
      </Box>

      <Typography variant="caption" color="text.secondary" sx={{ display: 'block', textAlign: 'center', mt: 0.5 }}>
        <Link component="button" type="button" underline="hover" onClick={() => navigate(`/map?room=${view.here.vnum}`)}>
          Open full map
        </Link>
      </Typography>
    </Box>
  );
}
