import { useEffect, useRef, useState } from 'react';
import mermaid from 'mermaid';

mermaid.initialize({ startOnLoad: false, theme: 'dark', securityLevel: 'loose' });

let counter = 0;

/** Renders one mermaid diagram from its source. Falls back to the raw code on error. */
export function MermaidBlock({ code }: { code: string }) {
  const ref = useRef<HTMLDivElement>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let cancelled = false;
    const id = `mermaid-${(counter += 1)}`;
    mermaid
      .render(id, code)
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
