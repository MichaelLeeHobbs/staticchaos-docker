import { useEffect, useRef } from 'react';
import * as d3 from 'd3';
import { Box } from '@mui/material';

export interface RoomNode extends d3.SimulationNodeDatum {
  vnum: number;
  name: string;
  area: string;
}

type RoomLink = d3.SimulationLinkDatum<RoomNode>;

interface MapGraphProps {
  nodes: RoomNode[];
  links: RoomLink[];
}

const WIDTH = 1200;
const HEIGHT = 800;

export function MapGraph({ nodes, links }: MapGraphProps) {
  const svgRef = useRef<SVGSVGElement | null>(null);

  useEffect(() => {
    const svgEl = svgRef.current;
    if (!svgEl) return;

    const svg = d3.select(svgEl);
    svg.selectAll('*').remove();

    const color = d3.scaleOrdinal(d3.schemeCategory10);

    // Mutable copies so d3 can attach x/y/index without mutating props.
    const nodeData: RoomNode[] = nodes.map((n) => ({ ...n }));
    const linkData: RoomLink[] = links.map((l) => ({ ...l }));

    const root = svg.append('g');

    const linkSel = root
      .append('g')
      .attr('stroke', '#888')
      .attr('stroke-opacity', 0.4)
      .selectAll<SVGLineElement, RoomLink>('line')
      .data(linkData)
      .join('line')
      .attr('stroke-width', 1);

    const nodeSel = root
      .append('g')
      .attr('stroke', '#1a1a1a')
      .attr('stroke-width', 1)
      .selectAll<SVGCircleElement, RoomNode>('circle')
      .data(nodeData)
      .join('circle')
      .attr('r', 5)
      .attr('fill', (d) => color(d.area));

    nodeSel.append('title').text((d) => `${d.name} (#${d.vnum})`);

    const simulation = d3
      .forceSimulation<RoomNode>(nodeData)
      .force('charge', d3.forceManyBody().strength(-60))
      .force(
        'link',
        d3
          .forceLink<RoomNode, RoomLink>(linkData)
          .id((d) => d.vnum)
          .distance(40),
      )
      .force('center', d3.forceCenter(WIDTH / 2, HEIGHT / 2))
      .force('collide', d3.forceCollide(8));

    simulation.on('tick', () => {
      linkSel
        .attr('x1', (d) => (typeof d.source === 'object' ? (d.source.x ?? 0) : 0))
        .attr('y1', (d) => (typeof d.source === 'object' ? (d.source.y ?? 0) : 0))
        .attr('x2', (d) => (typeof d.target === 'object' ? (d.target.x ?? 0) : 0))
        .attr('y2', (d) => (typeof d.target === 'object' ? (d.target.y ?? 0) : 0));

      nodeSel.attr('cx', (d) => d.x ?? 0).attr('cy', (d) => d.y ?? 0);
    });

    const drag = d3
      .drag<SVGCircleElement, RoomNode>()
      .on('start', (event, d) => {
        if (!event.active) simulation.alphaTarget(0.3).restart();
        d.fx = d.x;
        d.fy = d.y;
      })
      .on('drag', (event, d) => {
        d.fx = event.x;
        d.fy = event.y;
      })
      .on('end', (event, d) => {
        if (!event.active) simulation.alphaTarget(0);
        d.fx = null;
        d.fy = null;
      });
    nodeSel.call(drag);

    const zoom = d3
      .zoom<SVGSVGElement, unknown>()
      .scaleExtent([0.1, 8])
      .on('zoom', (event: d3.D3ZoomEvent<SVGSVGElement, unknown>) => {
        root.attr('transform', event.transform.toString());
      });
    svg.call(zoom);

    return () => {
      simulation.stop();
      svg.on('.zoom', null);
      svg.selectAll('*').remove();
    };
  }, [nodes, links]);

  return (
    <Box sx={{ width: '100%', height: '72vh' }}>
      <svg
        ref={svgRef}
        viewBox={`0 0 ${WIDTH} ${HEIGHT}`}
        preserveAspectRatio="xMidYMid meet"
        style={{ width: '100%', height: '100%', cursor: 'grab', display: 'block' }}
      />
    </Box>
  );
}
