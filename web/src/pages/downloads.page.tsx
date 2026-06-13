import { useEffect, useState } from 'react';
import {
  Alert,
  Box,
  Button,
  Card,
  CardActions,
  CardContent,
  Chip,
  CircularProgress,
  Stack,
  Typography,
} from '@mui/material';
import DownloadIcon from '@mui/icons-material/Download';
import type { GmcpEntry } from '../lib/types';
import { gmcpUrl, loadGmcpIndex, loadText } from '../lib/data';
import { MarkdownView } from '../components/markdown-view';

const RECOMMENDED = 'StaticChaos.xml';

function formatBytes(bytes: number): string {
  const kb = bytes / 1024;
  if (kb < 1) return `${bytes} B`;
  return `${kb.toFixed(kb < 10 ? 1 : 0)} KB`;
}

function DownloadCard({ entry }: { entry: GmcpEntry }) {
  const recommended = entry.file === RECOMMENDED;
  return (
    <Card
      variant="outlined"
      sx={{
        height: '100%',
        display: 'flex',
        flexDirection: 'column',
        borderColor: recommended ? 'primary.main' : 'divider',
        borderWidth: recommended ? 2 : 1,
        boxShadow: recommended ? 3 : 0,
      }}
    >
      <CardContent sx={{ flexGrow: 1 }}>
        <Stack direction="row" spacing={1} alignItems="center" sx={{ mb: 1, flexWrap: 'wrap' }}>
          <Typography variant="h6" component="div" sx={{ fontFamily: 'monospace', wordBreak: 'break-all' }}>
            {entry.file}
          </Typography>
          {recommended && <Chip label="Recommended" color="primary" size="small" />}
        </Stack>
        <Typography variant="body2" color="text.secondary" sx={{ mb: 1 }}>
          {entry.desc}
        </Typography>
        <Typography variant="caption" color="text.disabled">
          {formatBytes(entry.bytes)}
        </Typography>
      </CardContent>
      <CardActions>
        <Button
          component="a"
          href={gmcpUrl(entry.file)}
          download
          startIcon={<DownloadIcon />}
          variant={recommended ? 'contained' : 'outlined'}
          size="small"
        >
          Download
        </Button>
      </CardActions>
    </Card>
  );
}

export function DownloadsPage() {
  const [entries, setEntries] = useState<GmcpEntry[] | null>(null);
  const [readme, setReadme] = useState<string | null>(null);
  const [err, setErr] = useState<string | null>(null);

  useEffect(() => {
    let live = true;
    Promise.all([loadGmcpIndex(), loadText('gmcp/README.md')])
      .then(([idx, md]) => {
        if (!live) return;
        setEntries(idx);
        setReadme(md);
      })
      .catch((e: unknown) => {
        if (live) setErr(e instanceof Error ? e.message : String(e));
      });
    return () => {
      live = false;
    };
  }, []);

  if (err) return <Alert severity="error">{err}</Alert>;
  if (entries === null || readme === null) {
    return (
      <Box sx={{ display: 'flex', justifyContent: 'center', py: 6 }}>
        <CircularProgress />
      </Box>
    );
  }

  const xmlEntries = entries.filter((e) => e.file.endsWith('.xml'));

  return (
    <Box>
      <Typography variant="h1" gutterBottom>
        Mudlet / GMCP
      </Typography>
      <Typography variant="body1" color="text.secondary" sx={{ mb: 3, maxWidth: 720 }}>
        These are Mudlet packages that hook into the server&rsquo;s GMCP data feed &mdash; powering live
        gauges (HP / mana / movement), the auto-mapper, and command tab-completion. Install one, log in,
        and your client lights up automatically.
      </Typography>

      {xmlEntries.length === 0 ? (
        <Alert severity="info" sx={{ mb: 3 }}>
          No downloadable packages are available right now.
        </Alert>
      ) : (
        <Box
          sx={{
            display: 'grid',
            gap: 2,
            gridTemplateColumns: { xs: '1fr', sm: 'repeat(2, 1fr)', md: 'repeat(3, 1fr)' },
            mb: 4,
          }}
        >
          {xmlEntries.map((entry) => (
            <DownloadCard key={entry.file} entry={entry} />
          ))}
        </Box>
      )}

      <Typography variant="h2" sx={{ fontSize: '1.6rem', mb: 1 }}>
        Installation
      </Typography>
      <MarkdownView source={readme} />
    </Box>
  );
}
