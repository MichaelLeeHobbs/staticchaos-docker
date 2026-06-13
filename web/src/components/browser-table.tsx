import { useMemo, useState } from 'react';
import {
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TablePagination,
  TableRow,
  Typography,
} from '@mui/material';

export interface BrowserColumn<T> {
  key: string;
  label: string;
  render: (row: T) => React.ReactNode;
  numeric?: boolean;
}

interface BrowserTableProps<T> {
  rows: T[];
  columns: BrowserColumn<T>[];
}

/**
 * Presentational, paginated table for the World Browser tabs. Rows are already
 * filtered by the caller; this component only handles pagination + layout.
 */
export function BrowserTable<T>({ rows, columns }: BrowserTableProps<T>) {
  const [page, setPage] = useState(0);
  const [rowsPerPage, setRowsPerPage] = useState(25);

  // Clamp the page if the filtered result set shrank below the current offset.
  const maxPage = Math.max(0, Math.ceil(rows.length / rowsPerPage) - 1);
  const safePage = Math.min(page, maxPage);

  const pageRows = useMemo(
    () => rows.slice(safePage * rowsPerPage, safePage * rowsPerPage + rowsPerPage),
    [rows, safePage, rowsPerPage],
  );

  return (
    <>
      <TableContainer>
        <Table size="small" stickyHeader>
          <TableHead>
            <TableRow>
              {columns.map((c) => (
                <TableCell key={c.key} align={c.numeric ? 'right' : 'left'} sx={{ fontWeight: 700, whiteSpace: 'nowrap' }}>
                  {c.label}
                </TableCell>
              ))}
            </TableRow>
          </TableHead>
          <TableBody>
            {pageRows.map((row, i) => (
              <TableRow key={safePage * rowsPerPage + i} hover>
                {columns.map((c) => (
                  <TableCell key={c.key} align={c.numeric ? 'right' : 'left'}>
                    {c.render(row)}
                  </TableCell>
                ))}
              </TableRow>
            ))}
            {pageRows.length === 0 && (
              <TableRow>
                <TableCell colSpan={columns.length}>
                  <Typography variant="body2" color="text.secondary" sx={{ py: 2, textAlign: 'center' }}>
                    No matching rows.
                  </Typography>
                </TableCell>
              </TableRow>
            )}
          </TableBody>
        </Table>
      </TableContainer>
      <TablePagination
        component="div"
        count={rows.length}
        page={safePage}
        onPageChange={(_e, p) => setPage(p)}
        rowsPerPage={rowsPerPage}
        onRowsPerPageChange={(e) => {
          setRowsPerPage(parseInt(e.target.value, 10));
          setPage(0);
        }}
        rowsPerPageOptions={[10, 25, 50, 100]}
      />
    </>
  );
}
