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
  Slider,
  Stack,
  TextField,
  ToggleButton,
  ToggleButtonGroup,
  Typography,
} from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import HomeIcon from '@mui/icons-material/Home';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';
import AutoFixHighIcon from '@mui/icons-material/AutoFixHigh';
import RestartAltIcon from '@mui/icons-material/RestartAlt';
import { loadWorld, loadSpawns } from '../lib/data';
import type { World } from '../lib/types';
import {
  drawGraph,
  computeGrid,
  computeSimpleGrid,
  gridDegenerate,
  type GraphHandle,
  type GraphLink,
  type GraphNode,
} from '../components/map-graph';

const DIRW: Record<string, string> = { N: 'north', E: 'east', S: 'south', W: 'west', U: 'up', D: 'down' };
const LS_KEY = 'sc-map-layout-v1'; // saved per-area node positions
type Layout = 'force' | 'grid';
type SavedPositions = Record<string, Record<string, [number, number]>>; // areaKey -> nodeId -> [x,y]
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
        const dest = world.rooms[String(to)];
        const oneWay = !dest || !Object.values(dest.exits).includes(+v);
        links.push({ source: `R${v}`, target: `R${to}`, dir, oneWay });
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
  const [spread, setSpread] = useState(1);
  const [history, setHistory] = useState<(string | null)[]>([]);

  const wrapRef = useRef<HTMLDivElement>(null);
  const svgRef = useRef<SVGSVGElement>(null);
  const handleRef = useRef<GraphHandle | null>(null);
  const pendingFocus = useRef<number | null>(null);
  const viewRef = useRef(view);
  viewRef.current = view;
  const historyRef = useRef(history);
  historyRef.current = history;
  const [resizeTick, setResizeTick] = useState(0);
  // saved node positions per area (in-memory, mirrored to localStorage)
  const positionsRef = useRef<Map<string, Record<string, [number, number]>>>(new Map());

  const persist = useCallback(() => {
    try {
      localStorage.setItem(LS_KEY, JSON.stringify(Object.fromEntries(positionsRef.current)));
    } catch {
      /* storage may be unavailable */
    }
  }, []);
  // drop the saved layout for the current view so the next draw recomputes it
  const forgetLayout = useCallback(() => {
    positionsRef.current.delete(viewRef.current ?? '__world__');
    persist();
  }, [persist]);

  // view changes go through navigate() so we can offer an in-map Back (the
  // browser back button would leave /map entirely — not what we want here).
  const navigate = useCallback((to: string | null) => {
    setHistory((h) => [...h, viewRef.current]);
    setSelected(null);
    setView(to);
  }, []);
  const goBack = useCallback(() => {
    const h = historyRef.current;
    if (!h.length) return;
    setHistory(h.slice(0, -1));
    setSelected(null);
    setView(h[h.length - 1] ?? null);
  }, []);

  // load any saved layouts before the first draw
  useEffect(() => {
    try {
      const raw = localStorage.getItem(LS_KEY);
      if (raw) positionsRef.current = new Map(Object.entries(JSON.parse(raw) as SavedPositions));
    } catch {
      /* ignore */
    }
  }, []);

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

  // Redraw on resize. Depends on `world` so it attaches AFTER the map (and
  // wrapRef) actually renders — on mount the component is still the loading
  // spinner, so wrapRef would be null and the observer would never attach.
  useEffect(() => {
    const el = wrapRef.current;
    if (!el) return;
    const ro = new ResizeObserver(() => setResizeTick((t) => t + 1));
    ro.observe(el);
    return () => ro.disconnect();
  }, [world]);

  const graph = useMemo(() => {
    if (!world) return null;
    if (view === null) {
      const { nodes, links } = buildWorld(world);
      const grid =
        layout === 'grid'
          ? computeSimpleGrid([...nodes].sort((a, b) => a.label.localeCompare(b.label)))
          : (null as Map<string, { x: number; y: number }> | null);
      return { nodes, links, dist: 180, charge: -1100 * spread, dirLabels: false, grid, gridFellBack: false };
    }
    const { nodes, links } = buildArea(world, view);
    const inArea = new Set(nodes.filter((n) => n.kind === 'room' && n.vnum != null).map((n) => n.vnum as number));
    const roomVnums = [...inArea];
    let grid: Map<string, { x: number; y: number }> | null = null;
    let gridFellBack = false;
    if (layout === 'grid') {
      const g = computeGrid(world, roomVnums, inArea, nodes);
      if (gridDegenerate(g, links)) gridFellBack = true; // too tangled for a grid → use force
      else grid = g;
    }
    return { nodes, links, dist: 95, charge: -340 * spread, dirLabels: true, grid, gridFellBack };
  }, [world, view, layout, spread]);

  const onNodeClick = useCallback(
    (n: GraphNode) => {
      if (n.kind === 'area' && n.areaId) {
        navigate(n.areaId);
      } else if (n.kind === 'portal' && n.toArea) {
        pendingFocus.current = n.toVnum ?? null;
        navigate(n.toArea);
      } else if (n.kind === 'room' && n.vnum != null) {
        setSelected(n.vnum);
      }
    },
    [navigate],
  );

  // (re)draw the graph whenever the view/layout/size changes. We read the
  // wrap's LIVE size here (it exists by the time graph is ready) rather than
  // trusting a measured-on-mount value.
  useEffect(() => {
    const wrap = wrapRef.current;
    if (!graph || !svgRef.current || !wrap) return;
    const key = view ?? '__world__';
    const saved = positionsRef.current.get(key);
    const initial = saved
      ? new Map(Object.entries(saved).map(([id, xy]) => [id, { x: xy[0], y: xy[1] }]))
      : null;
    const saveCurrent = () => {
      const h = handleRef.current;
      if (!h) return;
      positionsRef.current.set(key, h.getPositions());
      persist();
    };
    const handle = drawGraph(svgRef.current, graph.nodes, graph.links, {
      width: wrap.clientWidth || 900,
      height: wrap.clientHeight || 600,
      grid: graph.grid,
      dist: graph.dist,
      charge: graph.charge,
      dirLabels: graph.dirLabels,
      labelBelow: layout === 'grid',
      initialPositions: initial,
      onNodeClick,
      onBackgroundClick: () => setSelected(null),
      onLayoutChange: saveCurrent,
    });
    handleRef.current = handle;
    if (!initial) saveCurrent(); // persist the freshly-computed layout
    if (pendingFocus.current != null) {
      const v = pendingFocus.current;
      pendingFocus.current = null;
      handle.focus(`R${v}`);
      setSelected(v);
    }
    return () => handle.destroy();
  }, [graph, layout, view, onNodeClick, resizeTick, persist]);

  const focusRoom = (vnum: number) => {
    const dest = world?.rooms[String(vnum)];
    if (!dest) return;
    if (view !== dest.area) {
      pendingFocus.current = vnum;
      navigate(dest.area);
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
    if (area) navigate(area.id);
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
    <Box sx={{ display: 'flex', flexDirection: 'column', flex: 1, minHeight: 0 }}>
      <Stack direction="row" spacing={1.5} alignItems="center" sx={{ mb: 1.5, flexWrap: 'wrap', gap: 1 }}>
        <Button
          size="small"
          startIcon={<HomeIcon />}
          variant={view === null ? 'contained' : 'outlined'}
          onClick={() => navigate(null)}
        >
          World
        </Button>
        <Button
          size="small"
          startIcon={<ArrowBackIcon />}
          disabled={history.length === 0}
          onClick={goBack}
        >
          Back
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
          onChange={(_, v: Layout | null) => {
            if (v) {
              forgetLayout();
              setLayout(v);
            }
          }}
        >
          <ToggleButton value="force">Force</ToggleButton>
          <ToggleButton value="grid">Grid</ToggleButton>
        </ToggleButtonGroup>
        <Stack
          direction="row"
          spacing={1}
          alignItems="center"
          sx={{ minWidth: 150 }}
          title="How hard the layout pushes nodes apart"
        >
          <Typography variant="caption" color="text.secondary">
            Spread
          </Typography>
          <Slider
            size="small"
            defaultValue={1}
            min={0.3}
            max={3}
            step={0.1}
            valueLabelDisplay="auto"
            onChangeCommitted={(_, v) => {
              forgetLayout();
              setSpread(Array.isArray(v) ? (v[0] ?? 1) : v);
            }}
            sx={{ width: 100 }}
            disabled={layout === 'grid' && !graph?.gridFellBack}
          />
        </Stack>
        <Button size="small" startIcon={<AutoFixHighIcon />} onClick={() => handleRef.current?.relayout()}>
          Re-tidy
        </Button>
        <Button
          size="small"
          startIcon={<RestartAltIcon />}
          onClick={() => {
            forgetLayout();
            setResizeTick((t) => t + 1);
          }}
        >
          Reset
        </Button>
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
          : 'gold = recall · green ▸ = portal · amber = one-way · drag to move · Ctrl-click / Ctrl-drag to multi-select then drag the group · Re-tidy relaxes from here · layout saved in your browser'}
        {graph?.gridFellBack ? ' · ⚠ grid unsuitable for this area — showing Force' : ''}
      </Typography>

      <Box ref={wrapRef} sx={{ position: 'relative', flex: 1, minHeight: 0, width: '100%' }}>
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
              maxHeight: 'calc(100% - 16px)',
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
