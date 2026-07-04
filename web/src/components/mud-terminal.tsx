import { forwardRef, useEffect, useImperativeHandle, useRef, useState, useCallback } from 'react';
import { Box, Button, InputBase, Stack, Chip } from '@mui/material';
import { Terminal } from '@xterm/xterm';
import { FitAddon } from '@xterm/addon-fit';
import '@xterm/xterm/css/xterm.css';
import { MudConnection, type MudStatus, type GmcpMessage } from '../lib/mud-connection';

// The MUD (Static Chaos, a Merc derivative) does NOT echo normal input — a real
// telnet client relies on local echo. It masks passwords by sending IAC WILL
// ECHO. Our proxy consumes that negotiation and never forwards it, so the
// browser can't learn password mode from telnet. We therefore use a separate
// line-input box (local echo lives in the box, cleared on Enter) and detect
// password prompts heuristically from the trailing prompt text. This mirrors
// maldorne's own reference web client, which is also a line-input + read-only
// xterm against this same proxy.
const PASSWORD_PROMPT = /password[^\n]*:\s*$/i;
const ANSI = /\x1b\[[0-9;?]*[A-Za-z]/g;

const STATUS_LABEL: Record<MudStatus, string> = {
  connecting: 'Connecting…',
  connected: 'Connected',
  disconnected: 'Disconnected',
  error: 'Error',
};
const STATUS_COLOR: Record<MudStatus, 'default' | 'success' | 'warning' | 'error'> = {
  connecting: 'warning',
  connected: 'success',
  disconnected: 'default',
  error: 'error',
};

export interface MudTerminalProps {
  // Forwarded so a sibling panel can render GMCP + gray out on disconnect.
  // Kept as optional props (rather than lifting the whole connection out) to
  // keep this component the single owner of the MudConnection lifecycle.
  onGmcp?: (msg: GmcpMessage) => void;
  onStatus?: (status: MudStatus) => void;
}

// Imperative handle so a sibling (the GMCP panel) can inject text into the input
// line without lifting the input state out of this component — this keeps
// MudTerminal the sole owner of the connection lifecycle and the send path.
export interface MudTerminalHandle {
  // Replace the input line with `text`, focus it, and place the caret at the end
  // (does not auto-send). Used by the command/chant chips in the side-panel.
  insertInput: (text: string) => void;
}

export const MudTerminal = forwardRef<MudTerminalHandle, MudTerminalProps>(function MudTerminal(
  { onGmcp, onStatus } = {},
  ref,
) {
  const termHostRef = useRef<HTMLDivElement | null>(null);
  const termRef = useRef<Terminal | null>(null);
  const fitRef = useRef<FitAddon | null>(null);
  const connRef = useRef<MudConnection | null>(null);
  const inputRef = useRef<HTMLInputElement | null>(null);
  // Running tail of the current (unfinished) output line, for the password heuristic.
  const promptTailRef = useRef('');
  // Latest forwarding callbacks, so connect() can stay stable across renders.
  const onGmcpRef = useRef(onGmcp);
  onGmcpRef.current = onGmcp;
  const onStatusRef = useRef(onStatus);
  onStatusRef.current = onStatus;

  const [status, setStatus] = useState<MudStatus>('disconnected');
  const [detail, setDetail] = useState<string | undefined>(undefined);
  const [masked, setMasked] = useState(false);
  const [input, setInput] = useState('');
  const historyRef = useRef<string[]>([]);
  const historyIdxRef = useRef(-1);

  // Let the side-panel drop a "<command> " into the input and focus it. The
  // caret is placed after the value updates (next frame) so it lands at the end.
  useImperativeHandle(
    ref,
    () => ({
      insertInput: (text: string) => {
        setInput(text);
        requestAnimationFrame(() => {
          const el = inputRef.current;
          if (!el) return;
          el.focus();
          const end = el.value.length;
          try {
            el.setSelectionRange(end, end);
          } catch {
            /* some input types don't support selection ranges */
          }
        });
      },
    }),
    [],
  );

  // Create the terminal once, on mount.
  useEffect(() => {
    if (!termHostRef.current) return;
    const term = new Terminal({
      convertEol: true, // MUD sends bare \n; render as CR LF
      cursorBlink: false,
      disableStdin: true, // input goes through the line box below
      fontFamily: '"Cascadia Code", "Fira Code", Consolas, "DejaVu Sans Mono", monospace',
      fontSize: 14,
      scrollback: 5000,
      theme: { background: '#0b0b0e', foreground: '#cfcfd6', cursor: '#c9a24b' },
    });
    const fit = new FitAddon();
    term.loadAddon(fit);
    term.open(termHostRef.current);
    fit.fit();
    termRef.current = term;
    fitRef.current = fit;

    const ro = new ResizeObserver(() => {
      try {
        fit.fit();
      } catch {
        /* fit can throw if the element is detached mid-teardown */
      }
    });
    ro.observe(termHostRef.current);

    return () => {
      ro.disconnect();
      connRef.current?.disconnect();
      connRef.current = null;
      term.dispose();
      termRef.current = null;
      fitRef.current = null;
    };
  }, []);

  const handleText = useCallback((text: string) => {
    termRef.current?.write(text);
    // Update the trailing-prompt tracker for the password heuristic.
    let tail = promptTailRef.current + text;
    const nl = tail.lastIndexOf('\n');
    if (nl !== -1) tail = tail.slice(nl + 1);
    // Keep it bounded.
    if (tail.length > 200) tail = tail.slice(-200);
    promptTailRef.current = tail;
    setMasked(PASSWORD_PROMPT.test(tail.replace(ANSI, '')));
  }, []);

  const connect = useCallback(() => {
    if (connRef.current) return;
    const conn = new MudConnection({
      onText: handleText,
      onStatus: (s, d) => {
        setStatus(s);
        setDetail(d);
        onStatusRef.current?.(s);
        if (s === 'disconnected' || s === 'error') {
          connRef.current = null;
        }
      },
      // GMCP hook: forwarded to the side-panel (vitals, character, room, map).
      onGmcp: (msg) => onGmcpRef.current?.(msg),
    });
    connRef.current = conn;
    conn.connect();
    // Focus the input for immediate typing.
    setTimeout(() => inputRef.current?.focus(), 0);
  }, [handleText]);

  const disconnect = useCallback(() => {
    connRef.current?.disconnect();
    connRef.current = null;
  }, []);

  const submit = useCallback(() => {
    const line = input;
    setInput('');
    if (line.length > 0) {
      historyRef.current.push(line);
      if (historyRef.current.length > 100) historyRef.current.shift();
    }
    historyIdxRef.current = -1;
    connRef.current?.send(line);
    // The MUD doesn't echo input; reflect the command in the scrollback so the
    // session stays readable. Suppress while masked so passwords aren't shown.
    if (!masked) termRef.current?.write((line || '') + '\r\n');
    else termRef.current?.write('\r\n');
    setMasked(false);
  }, [input, masked]);

  const onKeyDown = useCallback(
    (e: React.KeyboardEvent<HTMLInputElement>) => {
      const hist = historyRef.current;
      if (e.key === 'Enter') {
        e.preventDefault();
        submit();
      } else if (e.key === 'ArrowUp') {
        e.preventDefault();
        if (hist.length === 0) return;
        historyIdxRef.current = Math.min(historyIdxRef.current + 1, hist.length - 1);
        setInput(hist[hist.length - 1 - historyIdxRef.current] ?? '');
      } else if (e.key === 'ArrowDown') {
        e.preventDefault();
        if (historyIdxRef.current <= 0) {
          historyIdxRef.current = -1;
          setInput('');
        } else {
          historyIdxRef.current -= 1;
          setInput(hist[hist.length - 1 - historyIdxRef.current] ?? '');
        }
      }
    },
    [submit],
  );

  const isConnected = status === 'connected' || status === 'connecting';

  return (
    <Box sx={{ display: 'flex', flexDirection: 'column', height: '100%', minHeight: 0 }}>
      <Stack direction="row" spacing={1.5} alignItems="center" sx={{ mb: 1, flexWrap: 'wrap' }}>
        {isConnected ? (
          <Button variant="outlined" color="inherit" size="small" onClick={disconnect}>
            Disconnect
          </Button>
        ) : (
          <Button variant="contained" size="small" onClick={connect}>
            Connect
          </Button>
        )}
        <Chip size="small" label={STATUS_LABEL[status]} color={STATUS_COLOR[status]} variant="outlined" />
        {detail && (
          <Box component="span" sx={{ color: 'text.secondary', fontSize: 13 }}>
            {detail}
          </Box>
        )}
      </Stack>

      <Box
        sx={{
          flex: 1,
          minHeight: 320,
          bgcolor: '#0b0b0e',
          border: '1px solid',
          borderColor: 'divider',
          borderRadius: 1,
          overflow: 'hidden',
          p: 1,
          '& .xterm': { height: '100%' },
        }}
      >
        <Box ref={termHostRef} sx={{ height: '100%', width: '100%' }} />
      </Box>

      <Stack
        direction="row"
        alignItems="center"
        spacing={1}
        sx={{
          mt: 1,
          px: 1.5,
          py: 0.5,
          bgcolor: '#121217',
          border: '1px solid',
          borderColor: 'divider',
          borderRadius: 1,
        }}
      >
        <Box component="span" sx={{ color: 'primary.main', fontFamily: 'monospace' }}>
          &gt;
        </Box>
        <InputBase
          inputRef={inputRef}
          fullWidth
          type={masked ? 'password' : 'text'}
          value={input}
          disabled={!isConnected}
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={onKeyDown}
          placeholder={isConnected ? 'Type a command and press Enter…' : 'Connect to start playing'}
          autoComplete="off"
          autoCorrect="off"
          autoCapitalize="off"
          spellCheck={false}
          sx={{ color: '#cfcfd6', fontFamily: 'monospace', fontSize: 14 }}
        />
      </Stack>
    </Box>
  );
});
