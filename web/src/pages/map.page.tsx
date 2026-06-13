import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import {
  Alert,
  Box,
  Button,
  Chip,
  CircularProgress,
  IconButton,
  Link,
  Paper,
  Stack,
  TextField,
  ToggleButton,
  ToggleButtonGroup,
  Typography,
} from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import HomeIcon from '@mui/icons-material/Home';
import { loadWorld, loadSpawns } from '../lib/data';
import type { World } from '../lib/types';
import {
  drawGraph,
  computeGrid,
  computeSimpleGrid,
  type GraphHandle,
  type GraphLink,
  type GraphNode,
} from '../components/map-graph';

const DIRW: Record<string, string> = { N: 'north', E: 'east', S: 'south', W: 'west', U: 'up', D: 'down' };
type Layout = 'force' | 'grid';
interface Enrich {
  roomMobs: Map<number, { name: string; level: number | null }[]>;
  roomItems: Map<number, string[]>;
}

/** Cross-area edges (area↔area) with a count, for the world overview. */
function areaLinks(world: World): GraphLink[] {
  const counts = new Map<string, number>();
  for (const r of Object.values(world.rooms)) {
    for (const to of Object.values(r.exits)) {
      const dest = world.rooms[String(to)];
      if (!dest || dest.area === r.area) continue;
      const key = [r.area, dest.area].sort().join('|');
      counts.set(key, (counts.get(key) ?? 0) + 1);
    }
  }
  return [...counts].map(([key, count]) => {
    const [source, target] = key.split('|');
    return { source: source as string, target: target as string, count };
  });
}

function buildWorld(world: World): { nodes: GraphNode[]; links: GraphLink[] } {
  const unreachable = new Set((world.reports.unreachableAreas as string[] | undefined) ?? []);
  const nodes: GraphNode[] = world.areas.map((a) => ({
    id: a.id,
    kind: 'area',
    areaId: a.id,
    label: a.name,
    r: Math.max(6, Math.min(26, 5 + Math.sqrt(a.roomCount) * 2)),
    color: unreachable.has(a.id) ? '#c0504d' : '#4b86d6',
    title: `${a.name} — ${a.roomCount} rooms${unreachable.has(a.id) ? ' (unreachable on foot)' : ''}`,
  }));
  const ids = new Set(world.areas.map((a) => a.id));
  const links = areaLinks(world).filter(
    (l) => ids.has(l.source as string) && ids.has(l.target as string),
  );
  return { nodes, links };
}

function buildArea(world: World, areaId: string): { nodes: GraphNode[]; links: GraphLink[] } {
  const roomVnums = Object.keys(world.rooms).filter((v) => world.rooms[v]?.area === areaId);
  const inArea = new Set(roomVnums.map(Number));
  const nodes: GraphNode[] = [];
  const links: GraphLink[] = [];
  const portals = new Set<string>();
  for (const v of roomVnums) {
    const r = world.rooms[v];
    if (!r) continue;
    nodes.push({
      id: `R${v}`,
      kind: 'room',
      vnum: +v,
      label: r.name || `#${v}`,
      r: 7,
      color: +v === world.recallVnum ? '#e0b341' : '#6f9ad6',
      title: `${r.name} #${v}`,
    });
  }
  for (const v of roomVnums) {
    const r = world.rooms[v];
    if (!r) continue;
    for (const dir of Object.keys(r.exits)) {
      const to = r.exits[dir];
      if (to == null) continue;
      if (inArea.has(to)) {
        links.push({ source: `R${v}`, target: `R${to}`, dir });
      } else {
        const pid = `P${to}`;
        const dest = world.rooms[String(to)];
        if (!portals.has(pid)) {
          const ta = dest ? dest.area : null;
          nodes.push({
            id: pid,
            kind: 'portal',
            toVnum: to,
            toArea: ta,
            r: 9,
            color: ta ? '#5aa17a' : '#7a4a4a',
            label: ta ? `▸ ${ta}` : `?? #${to}`,
            title: dest ? `${dest.name} — ${ta} #${to} (click to travel)` : `broken exit #${to}`,
          });
          portals.add(pid);
        }
        links.push({ source: `R${v}`, target: pid, dir, cross: true });
      }
    }
  }
  return { nodes, links };
}

export function MapPage() {
  const [world, setWorld] = useState<World | null>(null);
  const [err, setErr] = useState<string | null>(null);
  const [enrich, setEnrich] = useState<Enrich | null>(null);
  const [view, setView] = useState<string | null>(null); // null = world overview
  const [layout, setLayout] = useState<Layout>('force');
  const [selected, setSelected] = useState<number | null>(null);
  const [search, setSearch] = useState('');

  const wrapRef = useRef<HTMLDivElement>(null);
  const svgRef = useRef<SVGSVGElement>(null);
  const handleRef = useRef<GraphHandle | null>(null);
  const pendingFocus = useRef<number | null>(null);
  const [dims, setDims] = useState({ w: 900, h: 600 });

  useEffect(() => {
    let live = true;
    loadWorld()
      .then((w) => live && setWorld(w))
      .catch((e: unknown) => live && setErr(e instanceof Error ? e.message : String(e)));
    loadSpawns()
      .then((s) => {
        if (!live) return;
        const roomMobs = new Map<number, { name: string; level: number | null }[]>();
        for (const m of Object.values(s.mobSpawns)) {
          for (const rv of m.rooms) {
            const list = roomMobs.get(rv) ?? [];
            list.push({ name: m.name, level: m.level });
            roomMobs.set(rv, list);
          }
        }
        const roomItems = new Map<number, string[]>();
        for (const it of Object.values(s.itemSources)) {
          for (const src of it.sources) {
            const rv = (src as { room?: number }).room;
            if (typeof rv === 'number') {
              const list = roomItems.get(rv) ?? [];
              list.push(it.name);
              roomItems.set(rv, list);
            }
          }
        }
        setEnrich({ roomMobs, roomItems });
      })
      .catch(() => {
        /* enrichment is optional */
      });
    return () => {
      live = false;
    };
  }, []);

  // fill all available space: full container width, height down to the viewport bottom
  useEffect(() => {
    const el = wrapRef.current;
    if (!el) return;
    const measure = () => {
      const top = el.getBoundingClientRect().top;
      setDims({
        w: el.clientWidth || 900,
        h: Math.max(360, Math.floor(window.innerHeight - top - 10)),
      });
    };
    measure();
    const ro = new ResizeObserver(measure);
    ro.observe(el);
    window.addEventListener('resize', measure);
    return () => {
      ro.disconnect();
      window.removeEventListener('resize', measure);
    };
  }, []);

  const graph = useMemo(() => {
    if (!world) return null;
    if (view === null) {
      const { nodes, links } = buildWorld(world);
      const grid =
        layout === 'grid'
          ? computeSimpleGrid([...nodes].sort((a, b) => a.label.localeCompare(b.label)))
          : (null as Map<string, { x: number; y: number }> | null);
      return { nodes, links, dist: 150, charge: -800, zoom: 0.5, dirLabels: false, grid };
    }
    const { nodes, links } = buildArea(world, view);
    const inArea = new Set(nodes.filter((n) => n.kind === 'room' && n.vnum != null).map((n) => n.vnum as number));
    const roomVnums = [...inArea];
    const grid = layout === 'grid' ? computeGrid(world, roomVnums, inArea, nodes) : null;
    return {
      nodes,
      links,
      dist: 60,
      charge: -160,
      zoom: nodes.length > 120 ? 0.35 : 0.7,
      dirLabels: true,
      grid,
    };
  }, [world, view, layout]);

  const onNodeClick = useCallback((n: GraphNode) => {
    if (n.kind === 'area' && n.areaId) {
      setSelected(null);
      setView(n.areaId);
    } else if (n.kind === 'portal' && n.toArea) {
      pendingFocus.current = n.toVnum ?? null;
      setSelected(null);
      setView(n.toArea);
    } else if (n.kind === 'room' && n.vnum != null) {
      setSelected(n.vnum);
    }
  }, []);

  // (re)draw the graph whenever the view/layout/size changes
  useEffect(() => {
    if (!graph || !svgRef.current) return;
    const handle = drawGraph(svgRef.current, graph.nodes, graph.links, {
      width: dims.w,
      height: dims.h,
      grid: graph.grid,
      dist: graph.dist,
      charge: graph.charge,
      dirLabels: graph.dirLabels,
      labelBelow: layout === 'grid',
      zoom: graph.zoom,
      onNodeClick,
      onBackgroundClick: () => setSelected(null),
    });
    handleRef.current = handle;
    if (pendingFocus.current != null) {
      const v = pendingFocus.current;
      pendingFocus.current = null;
      handle.focus(`R${v}`);
      setSelected(v);
    }
    return () => handle.destroy();
  }, [graph, dims.w, dims.h, layout, view, onNodeClick]);

  const focusRoom = (vnum: number) => {
    const dest = world?.rooms[String(vnum)];
    if (!dest) return;
    if (view !== dest.area) {
      pendingFocus.current = vnum;
      setView(dest.area);
    } else {
      handleRef.current?.focus(`R${vnum}`);
      setSelected(vnum);
    }
  };

  const runSearch = () => {
    if (!world) return;
    const q = search.trim().toLowerCase();
    if (!q) return;
    const qv = q.replace('#', '');
    for (const [v, r] of Object.entries(world.rooms)) {
      if (v === qv || r.name.toLowerCase().includes(q)) {
        focusRoom(+v);
        return;
      }
    }
    const area = world.areas.find((a) => a.id.toLowerCase() === q || a.name.toLowerCase().includes(q));
    if (area) {
      setSelected(null);
      setView(area.id);
    }
  };

  if (err) return <Alert severity="error">{err}</Alert>;
  if (!world) {
    return (
      <Box sx={{ display: 'flex', justifyContent: 'center', py: 8 }}>
        <CircularProgress />
      </Box>
    );
  }

  const areaName = view ? (world.areas.find((a) => a.id === view)?.name ?? view) : null;
  const room = selected != null ? world.rooms[String(selected)] : null;

  return (
    <Box>
      <Stack direction="row" spacing={1.5} alignItems="center" sx={{ mb: 1.5, flexWrap: 'wrap', gap: 1 }}>
        <Button
          size="small"
          startIcon={<HomeIcon />}
          variant={view === null ? 'contained' : 'outlined'}
          onClick={() => {
            setSelected(null);
            setView(null);
          }}
        >
          World
        </Button>
        {areaName && (
          <Typography variant="body2" color="text.secondary">
            › {areaName} <span style={{ opacity: 0.6 }}>({view})</span>
          </Typography>
        )}
        <Box sx={{ flexGrow: 1 }} />
        <ToggleButtonGroup
          size="small"
          exclusive
          value={layout}
          onChange={(_, v: Layout | null) => v && setLayout(v)}
        >
          <ToggleButton value="force">Force</ToggleButton>
          <ToggleButton value="grid">Grid</ToggleButton>
        </ToggleButtonGroup>
        <TextField
          size="small"
          placeholder="search room or area… (name or #vnum)"
          value={search}
          onChange={(e) => setSearch(e.target.value)}
          onKeyDown={(e) => e.key === 'Enter' && runSearch()}
          sx={{ width: { xs: '100%', sm: 280 } }}
        />
      </Stack>

      <Typography variant="caption" color="text.secondary" sx={{ display: 'block', mb: 1 }}>
        {view === null
          ? `${world.areas.length} areas · blue = reachable, red = stranded · click an area to enter`
          : 'gold = recall · green ▸ = portal to another area (click to travel) · click a room for details'}
      </Typography>

      <Box ref={wrapRef} sx={{ position: 'relative', width: '100%', height: dims.h }}>
        <Paper variant="outlined" sx={{ width: '100%', height: '100%', overflow: 'hidden', bgcolor: '#0c0c10' }}>
          <svg ref={svgRef} width="100%" height="100%" style={{ display: 'block' }} />
        </Paper>

        {room && (
          <Paper
            elevation={6}
            sx={{
              position: 'absolute',
              top: 8,
              right: 8,
              width: 300,
              maxHeight: dims.h - 16,
              overflowY: 'auto',
              p: 2,
            }}
          >
            <Stack direction="row" justifyContent="space-between" alignItems="flex-start">
              <Typography variant="h6" sx={{ pr: 1 }}>
                {room.name || `#${selected}`}
              </Typography>
              <IconButton size="small" onClick={() => setSelected(null)}>
                <CloseIcon fontSize="small" />
              </IconButton>
            </Stack>
            <Typography variant="caption" color="text.secondary">
              room #{selected} · {room.area} · sector {room.sector}
            </Typography>

            <Typography variant="subtitle2" sx={{ mt: 1.5, color: 'primary.main' }}>
              Exits
            </Typography>
            {Object.keys(room.exits).length === 0 ? (
              <Typography variant="body2" color="text.secondary">
                none
              </Typography>
            ) : (
              Object.entries(room.exits).map(([dir, to]) => {
                const dest = world.rooms[String(to)];
                const sameArea = dest?.area === room.area;
                return (
                  <Box key={dir}>
                    <Link
                      component="button"
                      type="button"
                      underline="hover"
                      onClick={() => focusRoom(to)}
                      sx={{ textAlign: 'left' }}
                    >
                      {DIRW[dir] ?? dir} → {dest ? dest.name : `#${to}`}
                      {!sameArea && dest && (
                        <Box component="span" sx={{ color: '#5aa17a' }}>
                          {' '}
                          [{dest.area}]
                        </Box>
                      )}
                    </Link>
                  </Box>
                );
              })
            )}

            {enrich && selected != null && (enrich.roomMobs.get(selected)?.length ?? 0) > 0 && (
              <>
                <Typography variant="subtitle2" sx={{ mt: 1.5, color: 'primary.main' }}>
                  Creatures
                </Typography>
                <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 0.5 }}>
                  {enrich.roomMobs.get(selected)?.map((m, i) => (
                    <Chip key={`${m.name}-${i}`} size="small" label={`${m.name}${m.level != null ? ` (L${m.level})` : ''}`} />
                  ))}
                </Box>
              </>
            )}
            {enrich && selected != null && (enrich.roomItems.get(selected)?.length ?? 0) > 0 && (
              <>
                <Typography variant="subtitle2" sx={{ mt: 1.5, color: 'primary.main' }}>
                  Items
                </Typography>
                <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 0.5 }}>
                  {enrich.roomItems.get(selected)?.map((n, i) => (
                    <Chip key={`${n}-${i}`} size="small" variant="outlined" label={n} />
                  ))}
                </Box>
              </>
            )}
          </Paper>
        )}
      </Box>
    </Box>
  );
}
