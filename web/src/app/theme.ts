import { createTheme } from '@mui/material/styles';

// Dark, parchment-on-ink theme to match the MUD's tone.
export const theme = createTheme({
  palette: {
    mode: 'dark',
    primary: { main: '#c9a24b' },
    secondary: { main: '#7fae7f' },
    background: { default: '#0e0e11', paper: '#17171c' },
  },
  typography: {
    fontSize: 14,
    h1: { fontSize: '2rem' },
    h2: { fontSize: '1.5rem' },
    h3: { fontSize: '1.2rem' },
  },
  components: {
    MuiCssBaseline: {
      styleOverrides: { body: { scrollbarColor: '#3a3a42 #17171c' } },
    },
  },
});
