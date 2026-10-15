# DataTable — Week 1 Progress Report

**Contributor:** primah

### Primah Mukhaye — Data Model

- **Completed work:** Implemented the foundational `Cell`, `Row`, and `Column`
  data-model types. `Cell` stores null, boolean, signed and unsigned integer,
  floating-point, or string values, with typed inspection/access and text
  formatting. `Row` owns an ordered sequence of cells; `Column` owns an
  ordered sequence of cells and validates its required name. Both provide
  size/empty queries, checked indexed access, append, and clear operations.

- **Files changed:** [`cell.hpp`](../../include/datatable/cell.hpp),
  [`cell.cpp`](../../src/cell.cpp), [`row.hpp`](../../include/datatable/row.hpp),
  [`row.cpp`](../../src/row.cpp), [`column.hpp`](../../include/datatable/column.hpp),
  [`column.cpp`](../../src/column.cpp), and focused tests in
  [`test_cell.cpp`](../../tests/test_cell.cpp),
  [`test_row.cpp`](../../tests/test_row.cpp), and
  [`test_column.cpp`](../../tests/test_column.cpp).

- **Tests and validation:** Added focused coverage for supported cell types,
  null and string handling, formatting and invalid typed access; row and
  column construction, mutation, clearing, and bounds errors; and empty column
  names. The tests are registered as `DataTable.cell`, `DataTable.row`, and
  `DataTable.column` in [`tests/CMakeLists.txt`](../../tests/CMakeLists.txt).
  CMake Tools could not configure the project during this report update, so
  these tests were not executed.
- **Open decisions or blockers:** Agree with Elisha on how these standalone
  data-model types integrate with `DataTable`, including ownership and
  lifetime, supported value types, and invalid-access behavior. Table-level
  row/column integration remains pending that shared contract.

#### Class structure and implementation

Primah's work is organized around three data-model classes:

- **`Cell`:** stores one value in private `value_`, a `CellValue` variant.
  The variant supports null, `bool`, signed and unsigned integers, `double`,
  and `std::string`. Public constructors create supported values; `isNull()`,
  `value()`, `holds<T>()`, `get<T>()`, `getIf<T>()`, and `toString()` inspect,
  retrieve, or format the value. The value member is private so callers use
  checked access rather than changing the variant representation directly.
- **`Row`:** stores an ordered sequence of cells in private `cells_`.
  Public constructors create empty or initialized rows, while `size()`,
  `empty()`, `cells()`, `at()`, `operator[]`, `append()`, and `clear()` inspect
  or modify the sequence. Keeping the vector private preserves ownership and
  gives the class control over indexed access and bounds errors.
- **`Column`:** stores a column label in private `name_` and its ordered cell
  values in private `cells_`. Public constructors, `name()`, `size()`,
  `empty()`, `cells()`, `at()`, `operator[]`, `append()`, and `clear()` provide
  the usable column interface. The private `validateName()` function rejects
  empty names before the column becomes part of the data model.

These classes answer the design questions as follows: `Cell` was created to
represent one typed value, `Row` to represent one ordered record, and `Column`
to represent one named field. Their functions operate on their owned data.
Public functions expose safe operations needed by the table and algorithm
modules, while private members protect ownership, representation, and
validation rules.




