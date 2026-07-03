import * as d3 from 'd3';
import type { World } from '../lib/types';

// d3 force-graph engine for the world map. Framework-agnostic: drawGraph() owns
// the <svg> imperatively; the page passes nodes/links + click handlers and gets
// back a handle to focus a node or tear down. Ported from world-maps/world-map.html.

export interface GraphNode extends d3.SimulationNodeDatum {
  id: string;
  label: string;
  r: number;
  color: string;
  title?: string;
  kind?: 'area' | 'room' | 'portal';
  vnum?: number;
  toVnum?: number;
  toArea?: string | null;
  areaId?: string;
}
export interface GraphLink extends d3.SimulationLinkDatum<GraphNode> {
  dir?: string;
  cross?: boolean;
  count?: number;
  oneWay?: boolean;
}
interface DrawOpts {
  width: number;
  height: number;
  grid?: Map<string, { x: number; y: number }> | null;
  dist?: number;
  charge?: number;
  dirLabels?: boolean;
  labelBelow?: boolean;
  zoom?: number;
  // if given, use these node positions verbatim (skip auto-layout)
  initialPositions?: Map<string, { x: number; y: number }> | null;
  onNodeClick?: (n: GraphNode) => void;
  onBackgroundClick?: () => void;
  // fired after the user changes positions (drag / box-move / re-tidy) so the
  // page can persist the layout
  onLayoutChange?: () => void;
}
export interface GraphHandle {
  destroy: () => void;
  focus: (id: string) => void;
  // relax the current arrangement with the force sim (manually-placed/pinned
  // nodes stay put as anchors)
  relayout: () => void;
  // current node positions, for persistence
  getPositions: () => Record<string, [number, number]>;
}

const trunc = (s: string) => (s.length > 16 ? `${s.slice(0, 15)}…` : s);

const GRID_SP = 160; // compass-grid cell spacing (px)

// --- edge-crossing count, for the multi-restart "least tangled" layout ---
const cw = (xi: number, yi: number, xj: number, yj: number, xk: number, yk: number) =>
  (xk - xi) * (yj - yi) - (xj - xi) * (yk - yi);
function segmentsCross(a: GraphNode, b: GraphNode, c: GraphNode, d: GraphNode): boolean {
  const ax = a.x ?? 0, ay = a.y ?? 0, bx = b.x ?? 0, by = b.y ?? 0;
  const cx = c.x ?? 0, cy = c.y ?? 0, dx = d.x ?? 0, dy = d.y ?? 0;
  const d1 = cw(cx, cy, dx, dy, ax, ay);
  const d2 = cw(cx, cy, dx, dy, bx, by);
  const d3a = cw(ax, ay, bx, by, cx, cy);
  const d4 = cw(ax, ay, bx, by, dx, dy);
  return ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) && ((d3a > 0 && d4 < 0) || (d3a < 0 && d4 > 0));
}
function countCrossings(links: GraphLink[]): number {
  let n = 0;
  for (let i = 0; i < links.length; i++) {
    const li = links[i];
    if (!li) continue;
    const a = li.source as GraphNode;
    const b = li.target as GraphNode;
    for (let j = i + 1; j < links.length; j++) {
      const lj = links[j];
      if (!lj) continue;
      const c = lj.source as GraphNode;
      const d = lj.target as GraphNode;
      if (a === c || a === d || b === c || b === d) continue;
      if (segmentsCross(a, b, c, d)) n++;
    }
  }
  return n;
}

/* The compass-grid degrades badly on shortcut/maze areas (it collapses toward a
 * line, with overlaps). Flag it degenerate when too many edges stretch well
 * beyond an adjacent cell, so the page can auto-fall-back to the force layout.
 * Links here still carry string ids (called before drawGraph resolves them). */
export function gridDegenerate(
  grid: Map<string, { x: number; y: number }>,
  links: GraphLink[],
): boolean {
  let long = 0;
  let total = 0;
  for (const l of links) {
    const a = grid.get(l.source as string);
    const b = grid.get(l.target as string);
    if (!a || !b) continue;
    total++;
    if (Math.max(Math.abs(a.x - b.x), Math.abs(a.y - b.y)) / GRID_SP > 1.6) long++;
  }
  return total > 4 && long / total > 0.34;
}

export function drawGraph(
  svgEl: SVGSVGElement,
  nodes: GraphNode[],
  links: GraphLink[],
  opts: DrawOpts,
): GraphHandle {
  const { width: W, height: H } = opts;
  const svg = d3.select(svgEl);
  svg.selectAll('*').remove();
  const g = svg.append('g');

  const zoom = d3
    .zoom<SVGSVGElement, unknown>()
    .scaleExtent([0.06, 6])
    .on('zoom', (e) => g.attr('transform', e.transform.toString()));
  svg.call(zoom);

  const link = g
    .append('g')
    .selectAll<SVGLineElement, GraphLink>('line')
    .data(links)
    .join('line')
    .attr('stroke', (d) => (d.cross ? '#5aa17a' : d.oneWay ? '#d98a3d' : '#39424f'))
    .attr('stroke-dasharray', (d) => (d.cross ? '4 3' : 'none'))
    .attr('stroke-width', (d) => Math.min(1 + (d.count ?? 1) * 0.3, 4) + (d.oneWay ? 0.6 : 0));

  const elabel = opts.dirLabels
    ? g
        .append('g')
        .selectAll<SVGTextElement, GraphLink>('text')
        .data(links)
        .join('text')
        .attr('fill', '#7f8a9b')
        .attr('font-size', 9)
        .attr('text-anchor', 'middle')
        .text((d) => d.dir ?? '')
    : null;

  let sim: d3.Simulation<GraphNode, GraphLink>;
  const selected = new Set<string>();
  const refreshSelection = () =>
    node
      .select('circle')
      .attr('stroke', (d) => (selected.has((d as GraphNode).id) ? '#ffd479' : '#0c0c10'))
      .attr('stroke-width', (d) => (selected.has((d as GraphNode).id) ? 2.6 : 1.2));

  // Cheap drag (no force re-run). Dragging a SELECTED node moves the whole
  // selection together; otherwise just that node. Node stays where dropped.
  const dragBehavior = d3
    .drag<SVGGElement, GraphNode>()
    .on('start', (_e, d) => {
      d.fx = d.x;
      d.fy = d.y;
    })
    .on('drag', (e, d) => {
      if (selected.has(d.id) && selected.size > 1) {
        for (const n of nodes) {
          if (!selected.has(n.id)) continue;
          n.fx = n.x = (n.x ?? 0) + e.dx;
          n.fy = n.y = (n.y ?? 0) + e.dy;
        }
      } else {
        d.fx = d.x = e.x;
        d.fy = d.y = e.y;
      }
      ticked();
    })
    .on('end', () => opts.onLayoutChange?.());

  const node = g
    .append('g')
    .selectAll<SVGGElement, GraphNode>('g')
    .data(nodes)
    .join('g')
    .style('cursor', 'pointer')
    .call(dragBehavior)
    .on('click', (e, d) => {
      e.stopPropagation();
      // Ctrl/Cmd/Shift-click toggles selection (for group drag); plain click navigates
      if (e.ctrlKey || e.metaKey || e.shiftKey) {
        if (selected.has(d.id)) selected.delete(d.id);
        else selected.add(d.id);
        refreshSelection();
        return;
      }
      opts.onNodeClick?.(d);
    });

  node
    .append('circle')
    .attr('r', (d) => d.r)
    .attr('fill', (d) => d.color)
    .attr('stroke', '#0c0c10')
    .attr('stroke-width', 1.2);

  const text = node
    .append('text')
    .attr('fill', '#cbd3e1')
    .attr('font-size', 11)
    .attr('paint-order', 'stroke')
    .attr('stroke', '#0c0c10')
    .attr('stroke-width', 3)
    .text((d) => trunc(d.label));
  if (opts.labelBelow) {
    text
      .attr('x', 0)
      .attr('y', (d) => d.r + 11)
      .attr('text-anchor', 'middle');
  } else {
    text.attr('x', (d) => d.r + 3).attr('y', 4);
  }

  node.append('title').text((d) => d.title ?? d.label);

  // Ctrl/Cmd-drag on empty canvas = rubber-band box select (d3.zoom ignores
  // ctrl-drag by default, so it won't pan). Plain click on canvas clears.
  let boxStart: [number, number] | null = null;
  let boxRect: d3.Selection<SVGRectElement, unknown, null, undefined> | null = null;
  svg.on('mousedown.box', (e: MouseEvent) => {
    if (!(e.ctrlKey || e.metaKey)) return;
    e.preventDefault();
    boxStart = d3.pointer(e, g.node());
    boxRect = g
      .append('rect')
      .attr('fill', 'rgba(255,212,121,0.12)')
      .attr('stroke', '#ffd479')
      .attr('stroke-dasharray', '4 3')
      .attr('pointer-events', 'none');
  });
  svg.on('mousemove.box', (e: MouseEvent) => {
    if (!boxStart || !boxRect) return;
    const p = d3.pointer(e, g.node());
    boxRect
      .attr('x', Math.min(boxStart[0], p[0]))
      .attr('y', Math.min(boxStart[1], p[1]))
      .attr('width', Math.abs(p[0] - boxStart[0]))
      .attr('height', Math.abs(p[1] - boxStart[1]));
  });
  svg.on('mouseup.box', (e: MouseEvent) => {
    if (!boxStart) return;
    const p = d3.pointer(e, g.node());
    const x0 = Math.min(boxStart[0], p[0]);
    const x1 = Math.max(boxStart[0], p[0]);
    const y0 = Math.min(boxStart[1], p[1]);
    const y1 = Math.max(boxStart[1], p[1]);
    for (const n of nodes) {
      const nx = n.x ?? 0;
      const ny = n.y ?? 0;
      if (nx >= x0 && nx <= x1 && ny >= y0 && ny <= y1) selected.add(n.id);
    }
    refreshSelection();
    boxRect?.remove();
    boxStart = null;
    boxRect = null;
  });
  svg.on('click', (e: MouseEvent) => {
    if (e.ctrlKey || e.metaKey || e.shiftKey) return;
    if (selected.size) {
      selected.clear();
      refreshSelection();
    }
    opts.onBackgroundClick?.();
  });

  const elPos = (d: GraphLink) => {
    const s = d.source as GraphNode;
    const t = d.target as GraphNode;
    const dx = (t.x ?? 0) - (s.x ?? 0);
    const dy = (t.y ?? 0) - (s.y ?? 0);
    const L = Math.hypot(dx, dy) || 1;
    return { x: (s.x ?? 0) + dx * 0.3 - (dy / L) * 6, y: (s.y ?? 0) + dy * 0.3 + (dx / L) * 6 };
  };

  const ticked = () => {
    link
      .attr('x1', (d) => (d.source as GraphNode).x ?? 0)
      .attr('y1', (d) => (d.source as GraphNode).y ?? 0)
      .attr('x2', (d) => (d.target as GraphNode).x ?? 0)
      .attr('y2', (d) => (d.target as GraphNode).y ?? 0);
    if (elabel) elabel.attr('x', (d) => elPos(d).x).attr('y', (d) => elPos(d).y);
    node.attr('transform', (d) => `translate(${d.x ?? 0},${d.y ?? 0})`);
  };

  // Set the <g> transform directly AND sync d3-zoom's stored __zoom. Going via
  // svg.call(zoom.transform, t) is unreliable across redraws: the reused <svg>
  // keeps its old __zoom, so when t matches it d3 emits no 'zoom' event and the
  // freshly-appended <g> never gets the transform (= graph stuck at identity,
  // top-left). Applying to <g> directly always works; __zoom keeps later
  // user pan/zoom continuous.
  const recenter = (k: number, cx = 0, cy = 0) => {
    const t = d3.zoomIdentity.translate(W / 2 - cx * k, H / 2 - cy * k).scale(k);
    g.attr('transform', t.toString());
    svg.property('__zoom', t);
  };

  // fit the rendered node bbox to the viewport, centred — robust regardless of
  // where the layout settled (replaces a fixed origin-centred zoom).
  const fitView = () => {
    if (!nodes.length) return;
    const xs = nodes.map((n) => n.x ?? 0).sort((a, b) => a - b);
    const ys = nodes.map((n) => n.y ?? 0).sort((a, b) => a - b);
    // trimmed bbox (2nd–98th percentile) so a stray flung node can't tank the zoom
    const q = (arr: number[], p: number) =>
      arr[Math.min(arr.length - 1, Math.max(0, Math.round((arr.length - 1) * p)))] ?? 0;
    const minx = q(xs, 0.02);
    const maxx = q(xs, 0.98);
    const miny = q(ys, 0.02);
    const maxy = q(ys, 0.98);
    const pad = 70;
    const k = Math.max(
      0.05,
      Math.min((W - pad * 2) / Math.max(1, maxx - minx), (H - pad * 2) / Math.max(1, maxy - miny), 1.4),
    );
    recenter(k, (minx + maxx) / 2, (miny + maxy) / 2);
  };

  // relax from current positions (manually-dragged/pinned nodes stay as anchors).
  // Powers the "Re-tidy" button: spread out overlaps without discarding the
  // human's arrangement.
  const relax = () => {
    const s = d3
      .forceSimulation<GraphNode, GraphLink>(nodes)
      .force('link', d3.forceLink<GraphNode, GraphLink>(links).id((d) => d.id).distance(opts.dist ?? 80).strength(0.4))
      .force('charge', d3.forceManyBody<GraphNode>().strength(opts.charge ?? -320).distanceMax(700).theta(0.85))
      .force('collide', d3.forceCollide<GraphNode>().radius((d) => d.r + 20).iterations(2))
      .stop();
    for (let i = 0; i < 300; i++) s.tick();
    ticked();
    fitView();
  };

  // resolves link endpoints (source/target string ids -> node objects)
  const resolveLinks = () => {
    sim = d3
      .forceSimulation<GraphNode, GraphLink>(nodes)
      .force('link', d3.forceLink<GraphNode, GraphLink>(links).id((d) => d.id).strength(0));
    sim.stop();
    sim.on('tick', ticked);
  };

  if (opts.initialPositions) {
    const pos = opts.initialPositions;
    for (const n of nodes) {
      const p = pos.get(n.id);
      if (p) {
        n.x = p.x;
        n.y = p.y;
      }
      n.fx = null;
      n.fy = null;
    }
    resolveLinks();
    ticked();
    fitView();
  } else if (opts.grid) {
    const grid = opts.grid;
    for (const n of nodes) {
      const p = grid.get(n.id);
      if (p) {
        n.x = n.fx = p.x;
        n.y = n.fy = p.y;
      }
    }
    resolveLinks();
    ticked();
    fitView();
  } else {
    const makeSim = () =>
      d3
        .forceSimulation<GraphNode, GraphLink>(nodes)
        .force(
          'link',
          d3.forceLink<GraphNode, GraphLink>(links).id((d) => d.id).distance(opts.dist ?? 80).strength(0.5),
        )
        // distanceMax keeps repulsion local so isolated nodes aren't flung to
        // infinity (which would blow up the fit). forceX/Y hold it centred.
        .force('charge', d3.forceManyBody<GraphNode>().strength(opts.charge ?? -320).distanceMax(700).theta(0.8))
        .force('collide', d3.forceCollide<GraphNode>().radius((d) => d.r + 20).iterations(2))
        .force('x', d3.forceX<GraphNode>(0).strength(0.11))
        .force('y', d3.forceY<GraphNode>(0).strength(0.11))
        .stop();

    // Multi-restart: settle from several random starts and keep the layout with
    // the fewest edge crossings (crossing-minimisation is NP-hard, so this just
    // picks the best of N tries). Skip the O(E^2) count on very dense graphs.
    const iters = Math.min(550, 220 + nodes.length * 2);
    const restarts = nodes.length > 250 ? 2 : nodes.length > 120 ? 3 : 5;
    const evalCrossings = links.length <= 700;
    const span = Math.max(400, Math.sqrt(nodes.length) * 140);
    let best: { x: number; y: number }[] | null = null;
    let bestC = Infinity;
    for (let r = 0; r < restarts; r++) {
      for (const n of nodes) {
        n.x = (Math.random() - 0.5) * span;
        n.y = (Math.random() - 0.5) * span;
        n.vx = 0;
        n.vy = 0;
        n.fx = null;
        n.fy = null;
      }
      const s = makeSim();
      for (let i = 0; i < iters; i++) s.tick();
      if (!evalCrossings) {
        best = nodes.map((n) => ({ x: n.x ?? 0, y: n.y ?? 0 }));
        break;
      }
      const c = countCrossings(links);
      if (c < bestC) {
        bestC = c;
        best = nodes.map((n) => ({ x: n.x ?? 0, y: n.y ?? 0 }));
      }
    }
    if (best) {
      nodes.forEach((n, i) => {
        const p = best?.[i];
        if (p) {
          n.x = p.x;
          n.y = p.y;
          n.fx = null;
          n.fy = null;
        }
      });
    }
    // live (stopped) sim: resolves link endpoints + gives drag/focus a handle
    sim = makeSim();
    sim.on('tick', ticked);
    ticked();
    fitView();
  }

  return {
    destroy() {
      sim.stop();
      svg
        .on('.zoom', null)
        .on('click', null)
        .on('mousedown.box', null)
        .on('mousemove.box', null)
        .on('mouseup.box', null);
      svg.selectAll('*').remove();
    },
    focus(id) {
      const n = nodes.find((x) => x.id === id);
      if (!n) return;
      // just pan/zoom to it (don't re-run the sim or clobber selection rings)
      recenter(1.4, n.x ?? 0, n.y ?? 0);
    },
    relayout() {
      relax();
      opts.onLayoutChange?.();
    },
    getPositions() {
      const out: Record<string, [number, number]> = {};
      for (const n of nodes) out[n.id] = [Math.round(n.x ?? 0), Math.round(n.y ?? 0)];
      return out;
    },
  };
}

/* simple uniform grid for the world overview (areas have no compass exits): a
 * square-ish grid of evenly spaced cells, ordered as the nodes are passed in. */
export function computeSimpleGrid(
  nodes: GraphNode[],
  spacing = 170,
): Map<string, { x: number; y: number }> {
  const cols = Math.max(1, Math.ceil(Math.sqrt(nodes.length)));
  const px = new Map<string, { x: number; y: number }>();
  nodes.forEach((n, i) => {
    px.set(n.id, { x: (i % cols) * spacing, y: Math.floor(i / cols) * spacing });
  });
  return px;
}

/* compass-grid layout: BFS from the recall (or first) room, placing each room at
 * its neighbour's cell + the exit direction offset; collisions spiral outward. */
export function computeGrid(
  world: World,
  roomVnums: number[],
  inArea: Set<number>,
  nodes: GraphNode[],
): Map<string, { x: number; y: number }> {
  const DELTA: Record<string, [number, number]> = {
    N: [0, -1], S: [0, 1], E: [1, 0], W: [-1, 0], U: [1, -1], D: [-1, 1],
  };
  const SP = 160;
  const cells = new Set<string>();
  const gpos = new Map<string, [number, number]>();
  const claim = (gx: number, gy: number): [number, number] => {
    if (!cells.has(`${gx},${gy}`)) {
      cells.add(`${gx},${gy}`);
      return [gx, gy];
    }
    for (let r = 1; r < 80; r++) {
      for (let dx = -r; dx <= r; dx++) {
        for (let dy = -r; dy <= r; dy++) {
          if (Math.max(Math.abs(dx), Math.abs(dy)) !== r) continue;
          const k = `${gx + dx},${gy + dy}`;
          if (!cells.has(k)) {
            cells.add(k);
            return [gx + dx, gy + dy];
          }
        }
      }
    }
    return [gx, gy];
  };
  const first = roomVnums[0] ?? 0;
  const seedV = inArea.has(world.recallVnum) ? world.recallVnum : first;
  const seedId = `R${seedV}`;
  gpos.set(seedId, claim(0, 0));
  const queue = [seedId];
  const seen = new Set([seedId]);
  while (queue.length) {
    const id = queue.shift();
    if (!id || id[0] !== 'R') continue;
    const cell = gpos.get(id);
    if (!cell) continue;
    const room = world.rooms[id.slice(1)];
    if (!room) continue;
    for (const dir of Object.keys(room.exits)) {
      const to = room.exits[dir];
      if (to == null) continue;
      const nid = inArea.has(to) ? `R${to}` : `P${to}`;
      if (seen.has(nid)) continue;
      seen.add(nid);
      const d = DELTA[dir] ?? [1, 0];
      gpos.set(nid, claim(cell[0] + d[0], cell[1] + d[1]));
      queue.push(nid);
    }
  }
  let col = 0;
  const ys = [...gpos.values()].map((c) => c[1]);
  const baseY = (ys.length ? Math.max(...ys) : 0) + 2;
  for (const n of nodes) if (!gpos.has(n.id)) gpos.set(n.id, claim(col++, baseY));
  const px = new Map<string, { x: number; y: number }>();
  for (const [id, c] of gpos) px.set(id, { x: c[0] * SP, y: c[1] * SP });
  return px;
}
