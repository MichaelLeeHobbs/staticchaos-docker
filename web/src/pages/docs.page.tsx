import { useEffect, useState } from 'react';
import { Link as RouterLink, useParams } from 'react-router-dom';
import {
  Alert,
  Box,
  Chip,
  CircularProgress,
  Divider,
  Link,
  List,
  ListItemButton,
  ListItemText,
  Stack,
  Typography,
} from '@mui/material';
import type { DocEntry } from '../lib/types';
import { loadDoc, loadDocsIndex } from '../lib/data';
import { MarkdownView } from '../components/markdown-view';

const GROUP_ORDER = ['Guides', 'Reference', 'Atlas (areas)'] as const;

function DocsIndex() {
  const [entries, setEntries] = useState<DocEntry[] | null>(null);
  const [err, setErr] = useState<string | null>(null);

  useEffect(() => {
    let live = true;
    loadDocsIndex()
      .then((d) => {
        if (live) setEntries(d);
      })
      .catch((e: unknown) => {
        if (live) setErr(e instanceof Error ? e.message : String(e));
      });
    return () => {
      live = false;
    };
  }, []);

  if (err) return <Alert severity="error">{err}</Alert>;
  if (!entries) return <CircularProgress />;

  const byGroup = new Map<string, DocEntry[]>();
  for (const entry of entries) {
    const list = byGroup.get(entry.group);
    if (list) list.push(entry);
    else byGroup.set(entry.group, [entry]);
  }

  // Any groups not in the known order are appended after, alphabetically.
  const extraGroups = [...byGroup.keys()]
    .filter((g) => !GROUP_ORDER.includes(g as (typeof GROUP_ORDER)[number]))
    .sort();
  const orderedGroups = [...GROUP_ORDER, ...extraGroups];

  return (
    <Box>
      <Typography variant="h1" gutterBottom>
        Docs
      </Typography>
      {orderedGroups.map((group) => {
        const items = byGroup.get(group);
        if (!items || items.length === 0) return null;
        const isAtlas = group === 'Atlas (areas)';
        return (
          <Box key={group} sx={{ mb: 4 }}>
            <Typography variant="h2" sx={{ fontSize: '1.4rem', mb: 1 }}>
              {group}
            </Typography>
            <Divider sx={{ mb: 1.5 }} />
            {isAtlas ? (
              <Stack direction="row" spacing={1} useFlexGap sx={{
                flexWrap: "wrap"
              }}>
                {items.map((entry) => (
                  <Chip
                    key={entry.file}
                    label={entry.title}
                    component={RouterLink}
                    to={`/docs/${entry.file}`}
                    clickable
                    size="small"
                    variant="outlined"
                  />
                ))}
              </Stack>
            ) : (
              <List dense disablePadding>
                {items.map((entry) => (
                  <ListItemButton
                    key={entry.file}
                    component={RouterLink}
                    to={`/docs/${entry.file}`}
                  >
                    <ListItemText primary={entry.title} secondary={entry.file} />
                  </ListItemButton>
                ))}
              </List>
            )}
          </Box>
        );
      })}
    </Box>
  );
}

function DocView({ name }: { name: string }) {
  const [md, setMd] = useState<string | null>(null);
  const [err, setErr] = useState<string | null>(null);

  useEffect(() => {
    let live = true;
    setMd(null);
    setErr(null);
    loadDoc(name)
      .then((d) => {
        if (live) setMd(d);
      })
      .catch((e: unknown) => {
        if (live) setErr(e instanceof Error ? e.message : String(e));
      });
    return () => {
      live = false;
    };
  }, [name]);

  return (
    <Box>
      <Link component={RouterLink} to="/docs" sx={{ display: 'inline-block', mb: 2 }}>
        ← All docs
      </Link>
      <Typography variant="h2" sx={{ fontSize: '1rem', color: 'text.secondary', mb: 1 }}>
        {name}
      </Typography>
      {err ? (
        <Alert severity="error">{err}</Alert>
      ) : md === null ? (
        <CircularProgress />
      ) : (
        <MarkdownView source={md} />
      )}
    </Box>
  );
}

export function DocsPage() {
  const { name } = useParams<{ name: string }>();
  return name ? <DocView name={name} /> : <DocsIndex />;
}
