import { useEffect, useState } from 'react';
import { Box, Link, Typography } from '@mui/material';
import { loadJson } from '../lib/data';

/** Thin footer showing when the game data dumps were last generated. */
export function AppFooter() {
  const [generatedAt, setGeneratedAt] = useState<string | null>(null);

  useEffect(() => {
    let live = true;
    loadJson<{ generatedAt: string | null }>('data/meta.json')
      .then((m) => {
        if (live) setGeneratedAt(m.generatedAt);
      })
      .catch(() => {
        /* meta is optional */
      });
    return () => {
      live = false;
    };
  }, []);

  const when = generatedAt
    ? new Date(generatedAt).toLocaleDateString(undefined, { year: 'numeric', month: 'short', day: 'numeric' })
    : null;

  return (
    <Box
      component="footer"
      sx={{ py: 2, mt: 4, borderTop: '1px solid', borderColor: 'divider', textAlign: 'center' }}
    >
      <Typography variant="caption" sx={{
        color: "text.secondary"
      }}>
        Static Chaos companion — static, no login
        {when ? ` · game data generated ${when}` : ''} ·{' '}
        <Link href="https://www.mudlet.org/" target="_blank" rel="noopener" color="inherit">
          get Mudlet
        </Link>
      </Typography>
    </Box>
  );
}
