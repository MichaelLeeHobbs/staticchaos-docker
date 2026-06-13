import { HashRouter, Routes, Route } from 'react-router-dom';
import { ThemeProvider, CssBaseline } from '@mui/material';
import { theme } from './theme';
import { AppLayout } from '../components/app-layout';
import { HomePage } from '../pages/home.page';
import { DocsPage } from '../pages/docs.page';
import { MapPage } from '../pages/map.page';
import { BrowserPage } from '../pages/browser.page';
import { DownloadsPage } from '../pages/downloads.page';

export function App() {
  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <HashRouter>
        <Routes>
          <Route element={<AppLayout />}>
            <Route path="/" element={<HomePage />} />
            <Route path="/docs" element={<DocsPage />} />
            <Route path="/docs/:name" element={<DocsPage />} />
            <Route path="/map" element={<MapPage />} />
            <Route path="/browser" element={<BrowserPage />} />
            <Route path="/downloads" element={<DownloadsPage />} />
          </Route>
        </Routes>
      </HashRouter>
    </ThemeProvider>
  );
}
