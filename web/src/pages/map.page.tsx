import { useEffect, useMemo, useState } from 'react';
import {
  Alert,
  Box,
  CircularProgress,
  FormControl,
  InputLabel,
  MenuItem,
  Paper,
  Select,
  Stack,
  Typography,
} from '@mui/material';
import type { SelectChangeEvent } from '@mui/material';
import { loadWorld } from '../lib/data';
import type { World } from '../lib/types';
import { MapGraph, type RoomNode } from '../components/map-graph';

const ALL = '__all__';

export function MapPage() {
  const [world, setWorld] = useState<World | null>(null);
  const [err, setErr] = useState<string | null>(null);
  const [area, setArea] = useState<string>('');

  useEffect(() => {
    let live = true;
    loadWorld()
      .then((d) => {
        if (live) setWorld(d);
      })
      .catch((e: unknown) => {
        if (live) setErr(e instanceof Error ? e.message : String(e));
      });
    return () => {
      live = false;
    };
  }, []);

  // Default to the area with the most rooms once the world arrives.
  useEffect(() => {
    if (!world || area) return;
    const biggest = world.areas.reduce<World['areas'][number] | null>((best, a) => {
      if (!best || a.roomCount > best.roomCount) return a;
      return best;
    }, null);
    if (biggest) setArea(biggest.id);
  }, [world, area]);

  const { nodes, links } = useMemo(() => {
    if (!world || !area) return { nodes: [] as RoomNode[], links: [] };

    const allRooms = Object.values(world.rooms);
    const selected = area === ALL ? allRooms : allRooms.filter((r) => r.area === area);

    const nodeSet = new Set<number>(selected.map((r) => r.vnum));
    const nodeList: RoomNode[] = selected.map((r) => ({
      vnum: r.vnum,
      name: r.name,
      area: r.area,
    }));

    const seen = new Set<string>();
    const linkList: { source: number; target: number }[] = [];
    for (const r of selected) {
      for (const target of Object.values(r.exits)) {
        if (!nodeSet.has(target)) continue;
        // De-dupe undirected pairs.
        const key = r.vnum < target ? `${r.vnum}-${target}` : `${target}-${r.vnum}`;
        if (seen.has(key)) continue;
        seen.add(key);
        linkList.push({ source: r.vnum, target });
      }
    }

    return { nodes: nodeList, links: linkList };
  }, [world, area]);

  if (err) {
    return (
      <Box>
        <Typography variant="h1" gutterBottom>
          World Map
        </Typography>
        <Alert severity="error">{err}</Alert>
      </Box>
    );
  }

  if (!world || !area) {
    return (
      <Box>
        <Typography variant="h1" gutterBottom>
          World Map
        </Typography>
        <Box sx={{ display: 'flex', justifyContent: 'center', py: 8 }}>
          <CircularProgress />
        </Box>
      </Box>
    );
  }

  const sortedAreas = [...world.areas].sort((a, b) => b.roomCount - a.roomCount);

  const handleChange = (e: SelectChangeEvent<string>) => setArea(e.target.value);

  return (
    <Box>
      <Stack
        direction={{ xs: 'column', sm: 'row' }}
        spacing={2}
        alignItems={{ sm: 'center' }}
        justifyContent="space-between"
        sx={{ mb: 2 }}
      >
        <Typography variant="h1">World Map</Typography>
        <FormControl size="small" sx={{ minWidth: 260 }}>
          <InputLabel id="map-area-label">Area</InputLabel>
          <Select
            labelId="map-area-label"
            label="Area"
            value={area}
            onChange={handleChange}
          >
            <MenuItem value={ALL}>All areas (heavy)</MenuItem>
            {sortedAreas.map((a) => (
              <MenuItem key={a.id} value={a.id}>
                {a.name} ({a.roomCount})
              </MenuItem>
            ))}
          </Select>
        </FormControl>
      </Stack>

      <Typography variant="body2" color="text.secondary" sx={{ mb: 1 }}>
        {nodes.length} rooms, {links.length} connections. Drag nodes, scroll to zoom, drag
        background to pan.
      </Typography>

      <Paper variant="outlined" sx={{ overflow: 'hidden', bgcolor: 'background.default' }}>
        <MapGraph nodes={nodes} links={links} />
      </Paper>
    </Box>
  );
}
