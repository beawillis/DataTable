# DataTable — Week 2 Progress Report

**Reporting period:** Week 2
**Contributor:** Nantale Swabrah (25/U/08772/PS) — Sorting

## 1. Week 2 objective

Implement reusable sorting functionality for `DataTable` rows. Sorting must
support ascending and descending order, numeric and string values, column
selection, invalid-column handling, and row integrity so values from the same
record remain together.

## 2. Swabrah's work completed

### 2.1 Public sorting API

Updated [`sort.hpp`](../../include/datatable/sort.hpp) with:

- `SortOrder::ascending`;
- `SortOrder::descending`;
- `sortColumn()` for returning a sorted copy of a standalone column; and
- `sortByColumn()` for sorting `DataTable` rows by a selected column.

`sortByColumn()` accepts a table reference, a zero-based column index, and an
optional sort direction.

### 2.2 Table-row sorting

Implemented sorting in [`sort.cpp`](../../src/sort.cpp). The algorithm:

1. Validates the requested column through `DataTable::column()`.
2. Copies the table rows into a temporary vector.
3. Uses `std::stable_sort()` on the selected cell in each row.
4. Supports ascending and descending order.
5. Replaces the table row order through the controlled
   `DataTable::replaceRows()` operation.
6. Rebuilds every column from the reordered rows.

Because complete `Row` objects are moved rather than individual column cells,
values from different columns remain associated with the correct record.
Stable sorting also preserves the original order of rows with equal sort keys.

### 2.3 Comparison behavior

The comparison logic:

- compares signed integers, unsigned integers, booleans, and doubles
  numerically;
- compares strings alphabetically using `std::string` ordering;
- places null cells before non-null cells in ascending order;
- reverses the comparison for descending order; and
- gives mixed unsupported value combinations a deterministic variant-order
  comparison instead of silently treating unrelated types as equal.

### 2.4 Tests

Extended [`test_sort.cpp`](../../tests/test_sort.cpp) to verify:

- integer ascending sorting;
- integer descending sorting;
- numeric ordering such as `2, 10, 100`;
- string sorting;
- duplicate sort values with stable ordering;
- sorting an empty table;
- sorting a table while preserving related values across columns; and
- invalid column-index rejection.

The existing standalone-column sorting behavior remains covered as well.
The test is registered as `DataTable.sort`.

## 3. Class structure and implementation

### `DataTable`

1. **What class is used?**
   `DataTable` is the shared table class whose rows are reordered.
2. **Why?**
   Sorting must operate on the central table model instead of creating a
   separate sorted-table representation.
3. **What data does it hold?**
   Its private members include `rows_`, `columns_`, `column_names_`, and
   `row_count_`.
4. **What functions operate on that data?**
   `rows()` provides the source rows, `column()` validates column selection,
   and `replaceRows()` replaces the row order and rebuilds synchronized
   columns.
5. **Why are members private and public?**
   The vectors remain private so sorting cannot leave rows and columns
   inconsistent. Public operations provide controlled access and validation.

### `Column`

1. **What class is used?**
   `Column` represents a named sequence of cells and supports standalone
   column sorting.
2. **Why?**
   It provides a reusable sorting operation for callers that only need a
   sorted column copy.
3. **What data does it hold?**
   Private `name_` and `cells_` members.
4. **What functions operate on that data?**
   `cells()` supplies values for copying, while `name()` preserves the column
   identity when `sortColumn()` returns the result.
5. **Why are members private and public?**
   Private storage protects the column state; public accessors expose only the
   data needed by the sorting algorithm.

### `Row`

1. **What class is used?**
   `Row` represents one complete table record.
2. **Why?**
   Sorting complete rows is what preserves relationships between columns.
3. **What data does it hold?**
   Private `cells_`, an ordered `std::vector<Cell>`.
4. **What functions operate on that data?**
   `at()` reads the selected sort cell and the row is copied or moved as one
   complete object.
5. **Why are members private and public?**
   Private storage prevents accidental structural changes; checked access lets
   the comparator inspect the selected cell safely.

### `Cell`

1. **What class is used?**
   `Cell` represents the value used as a sort key.
2. **Why?**
   Sorting needs one comparison path for strings, numeric values, booleans,
   and nulls.
3. **What data does it hold?**
   Private `value_` containing the `CellValue` variant.
4. **What functions operate on that data?**
   `isNull()`, `holds<T>()`, `get<T>()`, and `value()` allow the comparator to
   determine the stored type and compare its value.
5. **Why are members private and public?**
   The variant remains protected from direct structural corruption while the
   public typed accessors make comparison explicit.

## 4. Implemented object-oriented design

### Composition

Sorting uses the existing composition hierarchy:

- `DataTable` contains rows and columns.
- `Row` contains cells.
- `Column` contains cells.
- `sortByColumn()` reorders complete `Row` objects and then reconstructs the
  column view from those rows.

### Encapsulation

The sorting module does not access table storage directly. It uses public
`rows()`, `column()`, and `replaceRows()` operations. `DataTable` validates
row widths and maintains synchronization between its row and column views.
The comparison helpers remain private to `sort.cpp` because callers only need
the public sorting functions.

## 5. Design decisions and limitations

- Sorting mutates the supplied `DataTable` row order.
- Sorting is stable, so equal keys retain their original order.
- Column selection uses a zero-based column index.
- Invalid indices raise `std::out_of_range`.
- Empty tables are valid and remain unchanged when their selected column
  exists.
- Filtering is not part of this Week 2 sorting implementation.

## 6. Validation

```text
Build: successful
Sorting tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Next steps

1. Coordinate with the table-core and data-model owners before changing the
   shared sorting or row-replacement interfaces.
2. Extend sorting tests for additional missing-value and mixed-type policies
   if the team changes those agreed rules.
3. Implement filtering separately when it is assigned for a later week.
