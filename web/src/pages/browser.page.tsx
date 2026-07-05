import { useEffect, useMemo, useState } from 'react';
import {
  Alert,
  Box,
  Checkbox,
  CircularProgress,
  FormControlLabel,
  Link as MuiLink,
  Stack,
  Tab,
  Tabs,
  TextField,
  Typography,
} from '@mui/material';
import { Link as RouterLink, useSearchParams } from 'react-router-dom';
import { loadBestiary, loadItems, loadShops, loadSpawns } from '../lib/data';
import type { Item, Mob, Shop } from '../lib/types';
import { BrowserTable, type BrowserColumn } from '../components/browser-table';

type TabKey = 'items' | 'mobs' | 'shops' | 'spawns';
const TAB_KEYS: readonly TabKey[] = ['items', 'mobs', 'shops', 'spawns'];

interface SpawnRow {
  name: string;
  level: number | null;
  roomCount: number;
  rooms: number[];
}

const yn = (b: boolean) => (b ? '✓' : '✗');
// link a room vnum through to the World Map
const roomLink = (vnum: number) => (
  <MuiLink key={vnum} component={RouterLink} to={`/map?room=${vnum}`} sx={{ mr: 0.75, whiteSpace: 'nowrap' }}>
    #{vnum}
  </MuiLink>
);

/** Generic loader hook for a single tab's dataset (lazy, fired on first view). */
function useLazyData<T>(loader: () => Promise<T>, active: boolean) {
  const [data, setData] = useState<T | null>(null);
  const [err, setErr] = useState<string | null>(null);

  useEffect(() => {
    if (!active || data !== null || err !== null) return;
    let live = true;
    loader()
      .then((d) => {
        if (live) setData(d);
      })
      .catch((e: unknown) => {
        if (live) setErr(e instanceof Error ? e.message : String(e));
      });
    return () => {
      live = false;
    };
  }, [active, data, err, loader]);

  return { data, err };
}

function TabState({ err, loading }: { err: string | null; loading: boolean }) {
  if (err) return <Alert severity="error">{err}</Alert>;
  if (loading)
    return (
      <Box sx={{ display: 'flex', justifyContent: 'center', py: 6 }}>
        <CircularProgress />
      </Box>
    );
  return null;
}

function ItemsTab({ active, initialQuery }: { active: boolean; initialQuery?: string }) {
  const { data, err } = useLazyData(loadItems, active);
  const [q, setQ] = useState(initialQuery ?? '');
  const [obtainableOnly, setObtainableOnly] = useState(false);

  const rows = useMemo<Item[]>(() => {
    if (!data) return [];
    const needle = q.trim().toLowerCase();
    return data.items.filter((it) => {
      if (obtainableOnly && !it.obtainable) return false;
      if (!needle) return true;
      return (
        it.name.toLowerCase().includes(needle) ||
        it.area.toLowerCase().includes(needle) ||
        it.type.toLowerCase().includes(needle)
      );
    });
  }, [data, q, obtainableOnly]);

  const columns: BrowserColumn<Item>[] = [
    { key: 'vnum', label: 'vnum', numeric: true, render: (r) => r.vnum },
    { key: 'name', label: 'name', render: (r) => r.name },
    { key: 'area', label: 'area', render: (r) => r.area },
    { key: 'type', label: 'type', render: (r) => r.type },
    { key: 'wear', label: 'wear', render: (r) => r.wear.join('/') },
    { key: 'stats', label: 'stats', render: (r) => r.affects.map((a) => a.stat).join(', ') },
    { key: 'obtainable', label: 'obtainable', render: (r) => yn(r.obtainable) },
    {
      key: 'source',
      label: 'source',
      render: (r) => {
        const s = r.sources[0];
        if (!s) return '';
        const room = typeof s.room === 'number' ? s.room : undefined;
        return room ? (
          <MuiLink component={RouterLink} to={`/map?room=${room}`}>
            {s.text ?? `#${room}`}
          </MuiLink>
        ) : (
          (s.text ?? '')
        );
      },
    },
  ];

  return (
    <Box>
      <Stack
        direction={{ xs: 'column', sm: 'row' }}
        spacing={2}
        sx={{
          alignItems: { sm: 'center' },
          mb: 2
        }}>
        <TextField
          label="Search name / area / type"
          size="small"
          value={q}
          onChange={(e) => setQ(e.target.value)}
          sx={{ flexGrow: 1, maxWidth: 420 }}
        />
        <FormControlLabel
          control={<Checkbox checked={obtainableOnly} onChange={(e) => setObtainableOnly(e.target.checked)} />}
          label="Obtainable only"
        />
      </Stack>
      <TabState err={err} loading={!data && !err} />
      {data && <BrowserTable rows={rows} columns={columns} />}
    </Box>
  );
}

function MobsTab({ active, initialQuery }: { active: boolean; initialQuery?: string }) {
  const { data, err } = useLazyData(loadBestiary, active);
  const [q, setQ] = useState(initialQuery ?? '');

  const rows = useMemo<Mob[]>(() => {
    if (!data) return [];
    const needle = q.trim().toLowerCase();
    if (!needle) return data.mobs;
    return data.mobs.filter(
      (m) => m.name.toLowerCase().includes(needle) || m.area.toLowerCase().includes(needle),
    );
  }, [data, q]);

  const columns: BrowserColumn<Mob>[] = [
    { key: 'vnum', label: 'vnum', numeric: true, render: (r) => r.vnum },
    { key: 'name', label: 'name', render: (r) => r.name },
    { key: 'level', label: 'level', numeric: true, render: (r) => r.level },
    { key: 'area', label: 'area', render: (r) => r.area },
    { key: 'behaviour', label: 'behaviour', render: (r) => r.behaviour.join(', ') },
    { key: 'sighting', label: 'sighting', render: (r) => r.sighting },
  ];

  return (
    <Box>
      <TextField
        label="Search name / area"
        size="small"
        value={q}
        onChange={(e) => setQ(e.target.value)}
        sx={{ mb: 2, maxWidth: 420, width: '100%' }}
      />
      <TabState err={err} loading={!data && !err} />
      {data && <BrowserTable rows={rows} columns={columns} />}
    </Box>
  );
}

function ShopsTab({ active, initialQuery }: { active: boolean; initialQuery?: string }) {
  const { data, err } = useLazyData(loadShops, active);
  const [q, setQ] = useState(initialQuery ?? '');

  const rows = useMemo<Shop[]>(() => {
    if (!data) return [];
    const needle = q.trim().toLowerCase();
    if (!needle) return data.shops;
    return data.shops.filter(
      (s) => s.keeperName.toLowerCase().includes(needle) || s.area.toLowerCase().includes(needle),
    );
  }, [data, q]);

  const columns: BrowserColumn<Shop>[] = [
    { key: 'keeper', label: 'keeper', render: (r) => r.keeperName },
    { key: 'area', label: 'area', render: (r) => r.area },
    { key: 'room', label: 'room', numeric: true, render: (r) => r.room },
    { key: 'buys', label: 'buys', render: (r) => r.buys.join(', ') },
    { key: 'buyPct', label: 'buy%', numeric: true, render: (r) => r.profitBuy },
    { key: 'sellPct', label: 'sell%', numeric: true, render: (r) => r.profitSell },
    { key: 'hours', label: 'hours', render: (r) => r.hours },
  ];

  return (
    <Box>
      <TextField
        label="Search keeper / area"
        size="small"
        value={q}
        onChange={(e) => setQ(e.target.value)}
        sx={{ mb: 2, maxWidth: 420, width: '100%' }}
      />
      <TabState err={err} loading={!data && !err} />
      {data && <BrowserTable rows={rows} columns={columns} />}
    </Box>
  );
}

function SpawnsTab({ active, initialQuery }: { active: boolean; initialQuery?: string }) {
  const { data, err } = useLazyData(loadSpawns, active);
  const [q, setQ] = useState(initialQuery ?? '');

  const allRows = useMemo<SpawnRow[]>(() => {
    if (!data) return [];
    return Object.values(data.mobSpawns).map((s) => ({
      name: s.name,
      level: s.level,
      roomCount: s.rooms.length,
      rooms: s.rooms,
    }));
  }, [data]);

  const rows = useMemo<SpawnRow[]>(() => {
    const needle = q.trim().toLowerCase();
    if (!needle) return allRows;
    return allRows.filter((r) => r.name.toLowerCase().includes(needle));
  }, [allRows, q]);

  const columns: BrowserColumn<SpawnRow>[] = [
    { key: 'name', label: 'name', render: (r) => r.name },
    { key: 'level', label: 'level', numeric: true, render: (r) => r.level ?? '' },
    { key: 'roomCount', label: '#rooms', numeric: true, render: (r) => r.roomCount },
    {
      key: 'rooms',
      label: 'rooms',
      render: (r) => (
        <>
          {r.rooms.slice(0, 6).map(roomLink)}
          {r.rooms.length > 6 ? `+${r.rooms.length - 6}` : ''}
        </>
      ),
    },
  ];

  return (
    <Box>
      <TextField
        label="Search name"
        size="small"
        value={q}
        onChange={(e) => setQ(e.target.value)}
        sx={{ mb: 2, maxWidth: 420, width: '100%' }}
      />
      <TabState err={err} loading={!data && !err} />
      {data && <BrowserTable rows={rows} columns={columns} />}
    </Box>
  );
}

export function BrowserPage() {
  const [params] = useSearchParams();
  const paramTab = params.get('tab');
  const initialTab: TabKey = TAB_KEYS.includes(paramTab as TabKey) ? (paramTab as TabKey) : 'items';
  const initialQuery = params.get('q') ?? '';

  const [tab, setTab] = useState<TabKey>(initialTab);
  // Track which tabs have ever been viewed so their data persists across switches.
  const [seen, setSeen] = useState<Record<TabKey, boolean>>({
    items: initialTab === 'items',
    mobs: initialTab === 'mobs',
    shops: initialTab === 'shops',
    spawns: initialTab === 'spawns',
  });

  const onChange = (_e: React.SyntheticEvent, value: TabKey) => {
    setTab(value);
    setSeen((s) => (s[value] ? s : { ...s, [value]: true }));
  };
  const qFor = (k: TabKey) => (k === initialTab ? initialQuery : '');

  return (
    <Box>
      <Typography variant="h1" sx={{ mb: 2 }}>
        World Browser
      </Typography>
      <Tabs value={tab} onChange={onChange} sx={{ mb: 2, borderBottom: 1, borderColor: 'divider' }}>
        <Tab label="Items" value="items" />
        <Tab label="Mobs" value="mobs" />
        <Tab label="Shops" value="shops" />
        <Tab label="Spawns" value="spawns" />
      </Tabs>

      <Box hidden={tab !== 'items'}>{seen.items && <ItemsTab active={tab === 'items'} initialQuery={qFor('items')} />}</Box>
      <Box hidden={tab !== 'mobs'}>{seen.mobs && <MobsTab active={tab === 'mobs'} initialQuery={qFor('mobs')} />}</Box>
      <Box hidden={tab !== 'shops'}>{seen.shops && <ShopsTab active={tab === 'shops'} initialQuery={qFor('shops')} />}</Box>
      <Box hidden={tab !== 'spawns'}>{seen.spawns && <SpawnsTab active={tab === 'spawns'} initialQuery={qFor('spawns')} />}</Box>
    </Box>
  );
}
