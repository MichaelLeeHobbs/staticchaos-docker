import ReactMarkdown from 'react-markdown';
import remarkGfm from 'remark-gfm';
import { Box } from '@mui/material';
import { MermaidBlock } from './mermaid-block';

/**
 * Renders a markdown string (GFM tables + ```mermaid``` diagrams) with styling
 * that reads well on the dark theme. Used by the Docs and Downloads pages.
 */
export function MarkdownView({ source }: { source: string }) {
  return (
    <Box
      sx={{
        lineHeight: 1.6,
        wordBreak: 'break-word',
        '& h1': { mt: 2, mb: 1.5, fontSize: '1.9rem' },
        '& h2': { mt: 3, mb: 1, fontSize: '1.45rem', borderBottom: '1px solid', borderColor: 'divider', pb: 0.5 },
        '& h3': { mt: 2, mb: 0.5, fontSize: '1.15rem', color: 'primary.main' },
        '& a': { color: 'secondary.main' },
        '& code': { bgcolor: 'rgba(255,255,255,0.08)', px: 0.6, py: 0.2, borderRadius: 0.5, fontSize: '0.85em' },
        '& pre': { bgcolor: 'rgba(0,0,0,0.4)', p: 1.5, borderRadius: 1, overflowX: 'auto' },
        '& pre code': { bgcolor: 'transparent', px: 0 },
        '& blockquote': {
          borderLeft: '3px solid', borderColor: 'primary.main', m: 0, pl: 2, color: 'text.secondary',
        },
        '& table': { borderCollapse: 'collapse', width: '100%', my: 2, display: 'block', overflowX: 'auto' },
        '& th, & td': { border: '1px solid', borderColor: 'divider', px: 1, py: 0.5, textAlign: 'left' },
        '& th': { bgcolor: 'rgba(255,255,255,0.06)' },
        '& tr:nth-of-type(even)': { bgcolor: 'rgba(255,255,255,0.02)' },
        '& img': { maxWidth: '100%' },
      }}
    >
      <ReactMarkdown
        remarkPlugins={[remarkGfm]}
        components={{
          code({ className, children, ...props }) {
            if (/language-mermaid/.test(className ?? '')) {
              return <MermaidBlock code={String(children).replace(/\n$/, '')} />;
            }
            return (
              <code className={className} {...props}>
                {children}
              </code>
            );
          },
        }}
      >
        {source}
      </ReactMarkdown>
    </Box>
  );
}
