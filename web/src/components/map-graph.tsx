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
}
export interface DrawOpts {
  width: number;
  height: number;
  grid?: Map<string, { x: number; y: number }> | null;
  dist?: number;
  charge?: number;
  dirLabels?: boolean;
  labelBelow?: boolean;
  zoom?: number;
  onNodeClick?: (n: GraphNode) => void;
  onBackgroundClick?: () => void;
}
export interface GraphHandle {
  destroy: () => void;
  focus: (id: string) => void;
}

const trunc = (s: string) => (s.length > 16 ? `${s.slice(0, 15)}…` : s);

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
  svg.on('click', () => opts.onBackgroundClick?.());

  const link = g
    .append('g')
    .selectAll<SVGLineElement, GraphLink>('line')
    .data(links)
    .join('line')
    .attr('stroke', (d) => (d.cross ? '#5aa17a' : '#39424f'))
    .attr('stroke-dasharray', (d) => (d.cross ? '4 3' : 'none'))
    .attr('stroke-width', (d) => Math.min(1 + (d.count ?? 1) * 0.3, 4));

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

  const dragBehavior = d3
    .drag<SVGGElement, GraphNode>()
    .on('start', (e, d) => {
      if (!e.active) sim.alphaTarget(0.3).restart();
      d.fx = d.x;
      d.fy = d.y;
    })
    .on('drag', (e, d) => {
      d.fx = e.x;
      d.fy = e.y;
    })
    .on('end', (e, d) => {
      if (!e.active) sim.alphaTarget(0);
      if (!opts.grid) {
        d.fx = null;
        d.fy = null;
      }
    });

  const node = g
    .append('g')
    .selectAll<SVGGElement, GraphNode>('g')
    .data(nodes)
    .join('g')
    .style('cursor', 'pointer')
    .call(dragBehavior)
    .on('click', (e, d) => {
      e.stopPropagation();
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

  const recenter = (k: number, cx = 0, cy = 0) =>
    svg
      .transition()
      .duration(450)
      .call(zoom.transform, d3.zoomIdentity.translate(W / 2 - cx * k, H / 2 - cy * k).scale(k));

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

  if (opts.grid) {
    const grid = opts.grid;
    for (const n of nodes) {
      const p = grid.get(n.id);
      if (p) {
        n.x = n.fx = p.x;
        n.y = n.fy = p.y;
      }
    }
    // pinned positions; the strength-0 link force just resolves endpoints
    sim = d3
      .forceSimulation<GraphNode, GraphLink>(nodes)
      .force('link', d3.forceLink<GraphNode, GraphLink>(links).id((d) => d.id).strength(0));
    sim.stop();
    sim.on('tick', ticked);
    ticked();
    fitView();
  } else {
    sim = d3
      .forceSimulation<GraphNode, GraphLink>(nodes)
      .force(
        'link',
        d3.forceLink<GraphNode, GraphLink>(links).id((d) => d.id).distance(opts.dist ?? 80).strength(0.5),
      )
      // distanceMax keeps repulsion local so isolated nodes aren't flung to
      // infinity (which would blow up the fit). forceX/Y hold the graph centred.
      .force('charge', d3.forceManyBody<GraphNode>().strength(opts.charge ?? -320).distanceMax(700).theta(0.8))
      .force('collide', d3.forceCollide<GraphNode>().radius((d) => d.r + 20).iterations(2))
      .force('x', d3.forceX<GraphNode>(0).strength(0.11))
      .force('y', d3.forceY<GraphNode>(0).strength(0.11));
    // pre-settle silently (no DOM churn) for a detangled, immediately-fitted view;
    // drag reheats the sim afterwards.
    sim.stop();
    const iters = Math.min(500, 200 + nodes.length * 2);
    for (let i = 0; i < iters; i++) sim.tick();
    sim.on('tick', ticked);
    ticked();
    fitView();
  }

  return {
    destroy() {
      sim.stop();
      svg.on('.zoom', null).on('click', null);
      svg.selectAll('*').remove();
    },
    focus(id) {
      const n = nodes.find((x) => x.id === id);
      if (!n) return;
      node
        .select('circle')
        .attr('stroke', (d) => ((d as GraphNode).id === id ? '#ffffff' : '#0c0c10'))
        .attr('stroke-width', (d) => ((d as GraphNode).id === id ? 2.5 : 1.2));
      for (let i = 0; i < 60; i++) sim.tick();
      ticked();
      recenter(1.1, n.x ?? 0, n.y ?? 0);
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
