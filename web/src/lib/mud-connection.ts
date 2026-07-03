// MUD WebSocket connection for the browser "Play Now" client.
//
// Bridges the browser to the MUD (telnet TCP 4000) via a maldorne/mud-web-proxy
// sidecar. This module owns the proxy's WebSocket wire format so the React
// terminal component can stay thin. Wire format learned from the proxy source
// (github.com/maldorne/mud-web-proxy, src/connection.ts + telnet/handlers):
//
//   CLIENT -> PROXY
//     * A JSON string starting with '{' is a control message (ClientMessage).
//       To open the telnet connection we send { connect:1, mud:"<route>" }.
//       The proxy resolves the named route (host/port/encoding) server-side.
//     * Any other frame is forwarded verbatim to the MUD as user input.
//       (src/connection.ts handleClientMessage: `if (str[0] !== '{') forward`.)
//
//   PROXY -> CLIENT
//     * MUD output arrives as binary frames (the decoded telnet stream).
//     * The proxy strips/handles all telnet negotiation EXCEPT GMCP, which its
//       gmcp handler RE-WRAPS as `IAC SB GMCP <payload> IAC SE` and injects
//       back into the output stream (src/telnet/handlers/gmcp.ts handleSB).
//     * ECHO is consumed server-side and never forwarded (telnet/handlers/echo.ts),
//       so the browser cannot learn password mode from telnet — see the
//       heuristic in mud-terminal.tsx.
//
// latin1 note: our route is configured `encoding:"latin1"`, so the proxy
// latin1-decodes the MUD bytes and re-encodes them as UTF-8 on the wire
// (src/connection.ts sendToClient). That transform maps every original byte b
// to code point U+00bb, so the injected GMCP frame's telnet bytes arrive as
// U+00FF (IAC) U+00FA (SB) U+00C9 (GMCP=201) ... U+00FF U+00F0 (SE). We decode
// the frame as UTF-8 and strip those sequences at the string level below,
// handing the GMCP payload to an optional hook (future side-panel).

const IAC = 'ÿ';
const SB = 'ú';
const SE = 'ð';
const GMCP = 'É'; // telnet option 201

const WS_URL: string = import.meta.env.VITE_MUD_WS ?? 'ws://localhost:6200';
const MUD_ID: string = import.meta.env.VITE_MUD_ID ?? 'staticchaos';

export type MudStatus = 'connecting' | 'connected' | 'disconnected' | 'error';

export interface GmcpMessage {
  module: string; // e.g. "Char.Vitals"
  data: unknown; // parsed JSON payload, or the raw string if not JSON
}

export interface MudConnectionHandlers {
  onText: (text: string) => void;
  onStatus: (status: MudStatus, detail?: string) => void;
  onGmcp?: (msg: GmcpMessage) => void;
}

export class MudConnection {
  private ws: WebSocket | null = null;
  private handlers: MudConnectionHandlers;
  // UTF-8 stream decoder: the proxy sends UTF-8 bytes; a multi-byte char can
  // straddle two frames, so decode with { stream: true }.
  private decoder = new TextDecoder('utf-8', { fatal: false });
  // Holds an incomplete GMCP subnegotiation split across frames.
  private pending = '';
  private manualClose = false;

  constructor(handlers: MudConnectionHandlers) {
    this.handlers = handlers;
  }

  get connected(): boolean {
    return this.ws !== null && this.ws.readyState === WebSocket.OPEN;
  }

  connect(): void {
    if (this.ws) return;
    this.manualClose = false;
    this.pending = '';
    this.handlers.onStatus('connecting');

    let ws: WebSocket;
    try {
      ws = new WebSocket(WS_URL);
    } catch (err) {
      this.handlers.onStatus('error', err instanceof Error ? err.message : String(err));
      return;
    }
    ws.binaryType = 'arraybuffer';
    this.ws = ws;

    ws.onopen = () => {
      // Ask the proxy to open the telnet connection via our named route.
      // encoding is fixed by the route; we send it too as a belt-and-suspenders
      // guard against a misconfigured route.
      ws.send(JSON.stringify({ connect: 1, mud: MUD_ID, encoding: 'latin1', mccp: 0 }));
      this.handlers.onStatus('connected');
    };

    ws.onmessage = (event: MessageEvent) => {
      if (typeof event.data === 'string') {
        // Text frames beginning with '{' are proxy JSON control messages
        // (e.g. capacity errors); ignore them for the MVP.
        if (event.data.startsWith('{')) return;
        this.handleText(event.data);
        return;
      }
      // Binary frame: the decoded MUD output stream.
      const bytes = new Uint8Array(event.data as ArrayBuffer);
      this.handleText(this.decoder.decode(bytes, { stream: true }));
    };

    ws.onerror = () => {
      this.handlers.onStatus('error', 'Connection error — is the proxy reachable?');
    };

    ws.onclose = () => {
      this.ws = null;
      this.handlers.onStatus('disconnected', this.manualClose ? undefined : 'Connection closed by server.');
    };
  }

  disconnect(): void {
    this.manualClose = true;
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  }

  /** Send a line of user input to the MUD (telnet expects CR LF). */
  send(line: string): void {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(line + '\r\n');
    }
  }

  // Strip the injected GMCP frames (and any stray telnet command bytes),
  // forward GMCP payloads to the hook, and emit the remaining display text.
  private handleText(chunk: string): void {
    let s = this.pending + chunk;
    this.pending = '';

    let out = '';
    let i = 0;
    while (i < s.length) {
      if (s[i] !== IAC) {
        out += s[i];
        i++;
        continue;
      }
      const verb = s[i + 1];
      if (verb === undefined) {
        // Lone trailing IAC — buffer for the next frame.
        this.pending = s.slice(i);
        s = '';
        break;
      }
      if (verb === SB) {
        // Subnegotiation: IAC SB <opt> ... IAC SE
        const end = s.indexOf(IAC + SE, i + 2);
        if (end === -1) {
          // Incomplete — buffer from here for the next frame.
          this.pending = s.slice(i);
          break;
        }
        const opt = s[i + 2];
        const payload = s.slice(i + 3, end);
        if (opt === GMCP) this.emitGmcp(payload);
        i = end + 2; // skip past IAC SE
        continue;
      }
      // Any other IAC command (WILL/WONT/DO/DONT/GA/NOP...) — the proxy already
      // handles negotiation, so just drop the 2-byte sequence defensively.
      i += 2;
    }

    if (out) this.handlers.onText(out);
  }

  private emitGmcp(payload: string): void {
    if (!this.handlers.onGmcp) return;
    const sp = payload.indexOf(' ');
    const module = sp === -1 ? payload : payload.slice(0, sp);
    let data: unknown;
    if (sp !== -1) {
      const rest = payload.slice(sp + 1);
      try {
        data = JSON.parse(rest);
      } catch {
        data = rest;
      }
    }
    this.handlers.onGmcp({ module, data });
  }
}
