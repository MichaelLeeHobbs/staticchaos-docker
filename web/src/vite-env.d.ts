/// <reference types="vite/client" />

interface ImportMetaEnv {
  /** WebSocket endpoint of the mud-web-proxy sidecar. Dev: ws://localhost:6200; prod: wss://<host>/<path>. */
  readonly VITE_MUD_WS?: string;
  /** Named route the proxy resolves to the MUD container (MUD_ROUTES key). */
  readonly VITE_MUD_ID?: string;
}

interface ImportMeta {
  readonly env: ImportMetaEnv;
}
