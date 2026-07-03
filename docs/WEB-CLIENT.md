# Browser "Play Now" client

A React [`@xterm/xterm`](https://xtermjs.org/) terminal on the companion site
(`web/`, route `/play`) that lets people play Static Chaos in the browser with no
telnet client. Because browsers can't open raw TCP, the terminal connects over a
**WebSocket** to a [`maldorne/mud-web-proxy`](https://github.com/maldorne/mud-web-proxy)
sidecar, which bridges WebSocket ↔ telnet and handles telnet/GMCP negotiation to
the MUD on TCP 4000.

```
browser (xterm)  --wss/ws-->  mud-web-proxy  --telnet/TCP 4000-->  mud
   web/src/components/mud-terminal.tsx        docker service "mud"
   web/src/lib/mud-connection.ts
```

## Pieces

- **Client:** `web/src/lib/mud-connection.ts` (owns the proxy wire format) +
  `web/src/components/mud-terminal.tsx` (xterm + line input) + `web/src/pages/play.page.tsx`.
- **Proxy sidecar:** `mud-web-proxy` service in `docker-compose.yml` (dev) and
  `docker-compose.image.yml` (prod). It reaches the MUD over the internal Docker
  network via the named route `staticchaos` → `mud:4000`, encoding `latin1`.
  Raw 4000 is **not** published for the proxy's sake; the proxy talks to it
  internally.

## Wire format (from the proxy source)

Learned from `maldorne/mud-web-proxy` `src/connection.ts` and
`src/telnet/handlers/*`:

- **Client → proxy:** a JSON string beginning with `{` is a control message.
  We open the session with `{ "connect": 1, "mud": "staticchaos" }`; any other
  frame is forwarded verbatim to the MUD as user input.
- **Proxy → client:** MUD output arrives as binary WebSocket frames. The proxy
  strips/handles all telnet negotiation **except GMCP**, which it re-wraps as
  `IAC SB GMCP … IAC SE` and injects back into the stream. The client parses out
  those GMCP frames (payload handed to an optional hook; ignored for the MVP) and
  renders the rest.
- **latin1:** the route is `encoding: "latin1"`, so the proxy latin1-decodes the
  MUD and re-encodes UTF-8 on the wire. That maps each source byte `b` to code
  point `U+00bb`, so the injected GMCP telnet bytes arrive as `U+00FF/U+00FA/…`;
  the client strips them at the string level. See the header comment in
  `mud-connection.ts`.

## Configuration

Client (Vite env, `web/`):

| Var           | Default              | Meaning                                            |
| ------------- | -------------------- | -------------------------------------------------- |
| `VITE_MUD_WS` | `ws://localhost:6200` | Proxy WebSocket endpoint. Prod: `wss://<host>/mud`. |
| `VITE_MUD_ID` | `staticchaos`        | Named route the proxy resolves to the MUD.          |

Proxy (compose env): `WS_PORT=6200`, `DEFAULT_ENCODING=latin1`,
`ENABLE_LEGACY_ROUTING=false` (browser may only use the named route),
`ALLOWED_ORIGINS` (`MUD_PROXY_ALLOWED_ORIGINS`), `CHAT_ENABLED=false`,
`TLS_ENABLED=false`, and `MUD_ROUTES` pinning `staticchaos → mud:4000`.

## Run it locally

```bash
# 1. MUD + proxy sidecar
docker compose up --build          # publishes proxy on :6200, MUD on :4000

# 2. Web dev server (separate terminal)
cd web && pnpm install && pnpm dev  # http://localhost:5173  → open /play
```

The client defaults to `ws://localhost:6200`, so no env is needed for dev. Click
**Connect** on the Play Now page.

## Production: TLS / reverse proxy (required)

**An `https://` page cannot open a plain `ws://` socket** — the browser blocks
mixed content. So in production the site (served over https) must talk to the
proxy over **`wss://`**. We do **not** ship host-specific TLS here; terminate TLS
at your existing reverse proxy (Caddy / Traefik / nginx) and route to the proxy:

```
wss://<host>/mud   →   ws://mud-web-proxy:6200   (or 127.0.0.1:6200)
```

- In `docker-compose.image.yml` the proxy is bound to **`127.0.0.1:6200`** so
  only the host's reverse proxy can reach it; it is never exposed publicly as
  plain `ws://`.
- Set `MUD_PROXY_ALLOWED_ORIGINS` to the site origin (e.g.
  `https://staticchaos.example`) so the proxy rejects WebSocket upgrades from
  other origins.
- Point the web build at it: `VITE_MUD_WS=wss://<host>/mud` (and keep
  `VITE_MUD_ID=staticchaos`).

Example nginx location:

```nginx
location /mud {
    proxy_pass http://127.0.0.1:6200;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection "upgrade";
    proxy_set_header Host $host;
    proxy_set_header Origin $http_origin;
}
```

Example Caddy:

```
handle_path /mud* {
    reverse_proxy 127.0.0.1:6200
}
```

## Password echo

Static Chaos is a Merc derivative: it does **not** echo normal input (a telnet
client relies on local echo) and it masks passwords by sending `IAC WILL ECHO`
at password prompts (`echo_off_str` in `src/core/comm.c`). **The proxy consumes
the ECHO negotiation server-side and never forwards it to the browser**, so the
client can't learn password mode from telnet.

The client works around this: it uses a separate line-input box (local echo lives
in the box) and toggles it to a masked `password` field via a **heuristic** — it
watches the trailing prompt for `…password…:` (matches `Password:`,
`New password:`, `Retype password:`, `Give me a password for …:`). This is
best-effort; if a prompt changes, the mask could miss it.

> **Public-launch gate:** do not expose this client publicly until the pending
> **password-hashing** work lands. Passwords are currently stored plaintext
> (inherited upstream — see `README.docker.md`), and a public web login widens
> their exposure. Keep `/play` behind the TLS reverse proxy and an origin
> allowlist, and treat public launch as blocked on password hashing.
