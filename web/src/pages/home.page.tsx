import { useEffect, useState, type ReactNode } from 'react';
import {
  Alert,
  Box,
  Button,
  Card,
  CardActionArea,
  CardContent,
  Chip,
  Grid2 as Grid,
  Stack,
  Typography,
} from '@mui/material';
import { Link } from 'react-router-dom';
import MenuBookIcon from '@mui/icons-material/MenuBook';
import MapIcon from '@mui/icons-material/Map';
import TravelExploreIcon from '@mui/icons-material/TravelExplore';
import DownloadIcon from '@mui/icons-material/Download';
import RocketLaunchIcon from '@mui/icons-material/RocketLaunch';
import { loadBestiary, loadItems, loadWorld } from '../lib/data';

interface NavCard {
  to: string;
  title: string;
  desc: string;
  icon: ReactNode;
}

const CARDS: NavCard[] = [
  {
    to: '/docs',
    title: 'Docs & Guides',
    desc: 'Gearing, leveling, and reference guides for the realm.',
    icon: <MenuBookIcon fontSize="large" color="primary" />,
  },
  {
    to: '/map',
    title: 'Interactive World Map',
    desc: 'Explore every area and room with linked exits.',
    icon: <MapIcon fontSize="large" color="primary" />,
  },
  {
    to: '/browser',
    title: 'Item / Mob / Shop Browser',
    desc: 'Search items, mobs, and shops across all areas.',
    icon: <TravelExploreIcon fontSize="large" color="primary" />,
  },
  {
    to: '/downloads',
    title: 'Mudlet GMCP packages',
    desc: 'Ready-to-import Mudlet GMCP packages and configs.',
    icon: <DownloadIcon fontSize="large" color="primary" />,
  },
];

interface Stats {
  areas: number;
  rooms: number;
  items: number;
  mobs: number;
}

export function HomePage() {
  const [stats, setStats] = useState<Stats | null>(null);

  useEffect(() => {
    let live = true;
    Promise.all([loadWorld(), loadItems(), loadBestiary()])
      .then(([world, items, bestiary]) => {
        if (!live) return;
        setStats({
          areas: world.areas.length,
          rooms: Object.keys(world.rooms).length,
          items: items.items.length,
          mobs: bestiary.mobs.length,
        });
      })
      .catch(() => {
        // Stats are nice-to-have; cards render regardless.
      });
    return () => {
      live = false;
    };
  }, []);

  return (
    <Box>
      <Stack spacing={1.5} sx={{ mb: 4 }}>
        <Typography variant="h3" component="h1" fontWeight={700}>
          Static Chaos Companion
        </Typography>
        <Typography variant="h6" color="text.secondary" sx={{ maxWidth: 720 }}>
          A static, no-login companion for the Static Chaos MUD &mdash; browse
          guides, walk the world map, search the game&rsquo;s items, mobs, and
          shops, and grab Mudlet GMCP packages.
        </Typography>
        {stats && (
          <Stack direction="row" spacing={1} flexWrap="wrap" useFlexGap>
            <Chip label={`${stats.areas} areas`} size="small" />
            <Chip label={`${stats.rooms.toLocaleString()} rooms`} size="small" />
            <Chip label={`${stats.items.toLocaleString()} items`} size="small" />
            <Chip label={`${stats.mobs.toLocaleString()} mobs`} size="small" />
          </Stack>
        )}
        <Alert
          severity="info"
          icon={<RocketLaunchIcon />}
          action={
            <Button component={Link} to="/docs/GETTING-STARTED.md" color="inherit" size="small" variant="outlined">
              Start here
            </Button>
          }
          sx={{ maxWidth: 720, alignItems: 'center' }}
        >
          New to MUDs? The <strong>Getting Started</strong> guide takes you from zero to
          in-game — installing Mudlet and connecting included.
        </Alert>
      </Stack>

      <Grid container spacing={3}>
        {CARDS.map((card) => (
          <Grid key={card.to} size={{ xs: 12, sm: 6 }}>
            <Card sx={{ height: '100%' }} variant="outlined">
              <CardActionArea
                component={Link}
                to={card.to}
                sx={{ height: '100%' }}
              >
                <CardContent>
                  <Stack
                    direction="row"
                    spacing={2}
                    alignItems="flex-start"
                  >
                    {card.icon}
                    <Box>
                      <Typography variant="h6" component="h2" gutterBottom>
                        {card.title}
                      </Typography>
                      <Typography variant="body2" color="text.secondary">
                        {card.desc}
                      </Typography>
                    </Box>
                  </Stack>
                </CardContent>
              </CardActionArea>
            </Card>
          </Grid>
        ))}
      </Grid>
    </Box>
  );
}
