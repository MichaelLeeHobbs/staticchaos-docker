import { Box, Typography, Alert } from '@mui/material';
import { MudTerminal } from '../components/mud-terminal';

export function PlayPage() {
  return (
    <Box sx={{ display: 'flex', flexDirection: 'column', flex: 1, minHeight: 0 }}>
      <Typography variant="h1" gutterBottom>
        Play Now
      </Typography>
      <Typography variant="body1" color="text.secondary" sx={{ mb: 2, maxWidth: 760 }}>
        Play Static Chaos right here in your browser &mdash; no telnet client, no install. Hit
        <strong> Connect</strong>, then type commands in the input line below the screen. For the full
        experience (gauges, auto-map, GMCP), grab a Mudlet package from the Mudlet / GMCP page.
      </Typography>

      <Alert severity="info" sx={{ mb: 2, maxWidth: 760 }}>
        New here? Type <code>new</code> at the name prompt to create a character. Your password is
        hidden while typing at password prompts.
      </Alert>

      <Box sx={{ flex: 1, minHeight: 0 }}>
        <MudTerminal />
      </Box>
    </Box>
  );
}
