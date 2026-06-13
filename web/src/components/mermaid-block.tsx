import { useEffect, useRef, useState } from 'react';
import type { Mermaid } from 'mermaid';

// Load mermaid (large: ~1MB with its diagram renderers) only when a diagram is
// actually rendered, as a separate chunk — keeps it out of the initial bundle.
let mermaidPromise: Promise<Mermaid> | null = null;
function getMermaid(): Promise<Mermaid> {
  if (!mermaidPromise) {
    mermaidPromise = import('mermaid').then((m) => {
      m.default.initialize({ startOnLoad: false, theme: 'dark', securityLevel: 'loose' });
      return m.default;
    });
  }
  return mermaidPromise;
}

let counter = 0;

/** Renders one mermaid diagram from its source. Falls back to the raw code on error. */
export function MermaidBlock({ code }: { code: string }) {
  const ref = useRef<HTMLDivElement>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let cancelled = false;
    const id = `mermaid-${(counter += 1)}`;
    getMermaid()
      .then((mermaid) => mermaid.render(id, code))
      .then(({ svg }) => {
        if (!cancelled && ref.current) ref.current.innerHTML = svg;
      })
      .catch((e: unknown) => {
        if (!cancelled) setError(e instanceof Error ? e.message : String(e));
      });
    return () => {
      cancelled = true;
    };
  }, [code]);

  if (error) {
    return (
      <pre style={{ overflowX: 'auto', opacity: 0.8 }}>
        <code>{code}</code>
      </pre>
    );
  }
  return <div ref={ref} style={{ overflowX: 'auto', textAlign: 'center' }} />;
}
