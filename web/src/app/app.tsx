import { lazy, Suspense } from 'react';
import { HashRouter, Routes, Route } from 'react-router-dom';
import { ThemeProvider, CssBaseline, Box, CircularProgress } from '@mui/material';
import { theme } from './theme';
import { AppLayout } from '../components/app-layout';

// Route-level code splitting: d3 (map), the markdown/mermaid stack (docs,
// downloads) and the browser tables each load only when their route is visited.
const HomePage = lazy(() => import('../pages/home.page').then((m) => ({ default: m.HomePage })));
const PlayPage = lazy(() => import('../pages/play.page').then((m) => ({ default: m.PlayPage })));
const DocsPage = lazy(() => import('../pages/docs.page').then((m) => ({ default: m.DocsPage })));
const MapPage = lazy(() => import('../pages/map.page').then((m) => ({ default: m.MapPage })));
const BrowserPage = lazy(() => import('../pages/browser.page').then((m) => ({ default: m.BrowserPage })));
const DownloadsPage = lazy(() => import('../pages/downloads.page').then((m) => ({ default: m.DownloadsPage })));

function Loading() {
  return (
    <Box sx={{ display: 'flex', justifyContent: 'center', py: 8 }}>
      <CircularProgress />
    </Box>
  );
}

export function App() {
  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <HashRouter>
        <Suspense fallback={<Loading />}>
          <Routes>
            <Route element={<AppLayout />}>
              <Route path="/" element={<HomePage />} />
              <Route path="/play" element={<PlayPage />} />
              <Route path="/docs" element={<DocsPage />} />
              <Route path="/docs/:name" element={<DocsPage />} />
              <Route path="/map" element={<MapPage />} />
              <Route path="/browser" element={<BrowserPage />} />
              <Route path="/downloads" element={<DownloadsPage />} />
            </Route>
          </Routes>
        </Suspense>
      </HashRouter>
    </ThemeProvider>
  );
}
