import { AppBar, Toolbar, Typography, Button, Box, Container } from '@mui/material';
import { Link as RouterLink, useLocation, Outlet } from 'react-router-dom';

const NAV = [
  { to: '/', label: 'Home' },
  { to: '/docs', label: 'Docs' },
  { to: '/map', label: 'World Map' },
  { to: '/browser', label: 'Browser' },
  { to: '/downloads', label: 'Mudlet / GMCP' },
] as const;

export function AppLayout() {
  const { pathname } = useLocation();
  const isActive = (to: string) => (to === '/' ? pathname === '/' : pathname.startsWith(to));
  return (
    <Box sx={{ minHeight: '100vh', display: 'flex', flexDirection: 'column' }}>
      <AppBar position="sticky" color="default" enableColorOnDark>
        <Toolbar variant="dense" sx={{ gap: 0.5, flexWrap: 'wrap' }}>
          <Typography
            variant="h6"
            component={RouterLink}
            to="/"
            sx={{ mr: 2, color: 'primary.main', fontWeight: 700, textDecoration: 'none' }}
          >
            Static Chaos
          </Typography>
          {NAV.map((n) => (
            <Button
              key={n.to}
              component={RouterLink}
              to={n.to}
              size="small"
              color={isActive(n.to) ? 'primary' : 'inherit'}
            >
              {n.label}
            </Button>
          ))}
        </Toolbar>
      </AppBar>
      <Container maxWidth="lg" sx={{ py: 3, flex: 1 }}>
        <Outlet />
      </Container>
    </Box>
  );
}
