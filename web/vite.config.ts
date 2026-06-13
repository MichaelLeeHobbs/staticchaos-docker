import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// Static SPA served at the domain root on port 80.
export default defineConfig({
  plugins: [react()],
  build: { outDir: 'dist', chunkSizeWarningLimit: 1500 },
});
