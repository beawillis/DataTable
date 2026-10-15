# DataTable — Week 1 Progress Report

**Contributor:** nantale

### Nantale — Sorting and Filtering

- **Completed work:** Implemented non-mutating column sorting and filtering
  using the shared `Column` and `Cell` classes. Sorting supports ascending and
  descending order, and filtering supports predicates and exact value matches.
  Numeric cells are compared numerically, strings lexicographically, nulls
  before non-null values, and mixed types deterministically by variant order.
- **Files changed:** [`sort.hpp`](../../include/datatable/sort.hpp),
  [`sort.cpp`](../../src/sort.cpp), [`filter.hpp`](../../include/datatable/filter.hpp),
  [`filter.cpp`](../../src/filter.cpp), [`test_sort.cpp`](../../tests/test_sort.cpp),
  [`test_filter.cpp`](../../tests/test_filter.cpp), and
  [`tests/CMakeLists.txt`](../../tests/CMakeLists.txt).
- **Tests and validation:** Added tests for ascending and descending sorting,
  source-column preservation, exact filtering, predicate filtering, and
  filtered result contents. The project configured and built successfully.
  Sort tests passed. Some CTest executables were blocked from starting by the
  Windows Application Control policy in the environment; this is unrelated to
  compilation.
- **Open decisions or blockers:** Table-level row sorting and filtering remain
  dependent on the future `DataTable` row/column integration. The current Week
  1 API intentionally returns new columns and does not mutate the source.

#### Class structure and implementation notes

The sort/filter work is organized around the actual classes already used in the
project:

- `DataTable` is the main table object. It owns the schema and the tabular
  structure used by the rest of the program. Its `column_names_` member holds the
  table headings, and `row_count_` keeps the current table size. Public members
  such as `addColumn()` and `columnNames()` allow the program to inspect or extend
  the table structure, while the internal state stays private to preserve a valid
  table schema.
- `Column` represents one data column. It stores a `name_` and a `cells_`
  vector, meaning a column is both identified by its label and owns the values
  belonging to that field. The public API exposes `name()`, `size()`, `empty()`,
  `at()`, `append()`, and `clear()`, which allows the rest of the program to
  read or mutate a single column without needing to know how the values are
  stored internally. The private `validateName()` helper prevents invalid empty
  names.
- `Row` represents a single record. It owns a `cells_` vector, which means each
  row is responsible for its own sequence of field values. Public accessors such
  as `size()`, `cells()`, `at()`, `append()`, and `clear()` keep row handling
  simple and consistent. The private storage keeps the row's data safe from direct
  misuse.
- `Cell` is the actual value container. It holds a `CellValue` variant named
  `value_`, which can represent null, boolean, integer, unsigned integer,
  floating-point, or string data. Public methods like `isNull()`, `holds<T>()`,
  `get<T>()`, and `toString()` let the program inspect and compare values without
  exposing the internal variant storage.

These classes answer the key questions of the design:

1. What class did we create or design?
   The sort/filter module is organized around the existing `DataTable`, `Column`,
   `Row`, and `Cell` classes. We did not create a separate table model because
   the project already establishes one central DataTable structure.

2. Why?
   Sorting and filtering are operations on table data, not independent data
   stores. Reusing the project’s central classes keeps the program consistent,
   lets the logic work with real rows and columns, and avoids duplicating the
   same structure in multiple modules.

3. What data does it hold?
   `DataTable` holds column names and the current table size, `Column` holds a
   name and a list of cells, `Row` holds a list of cells for one record, and
   `Cell` holds one typed value.

4. What functions operate on that data?
   `DataTable` exposes schema and size queries, `Column` exposes access and
   mutation of a column, `Row` exposes access and mutation of a row, and `Cell`
   exposes null checks, typed reads, and string conversion. Sort and filter logic
   will use these APIs to locate a target column, compare cell values, and build a
   reordered or filtered result.

5. Why are some members private and others public?
   The public interface exposes the operations other modules need, while the data
   members remain private so the class can enforce valid state. For example,
   `Column` keeps `name_` and `cells_` private so it can validate names and control
   how cells are appended or accessed. This prevents invalid table states and keeps
   the rest of the project using the API instead of directly manipulating internal
   storage.

The Week 1 implementation uses a `Column` directly: `sortColumn()` copies its
cells, orders them, and returns a new named column; `filterColumn()` copies only
accepted cells; and `filterEquals()` supplies a common exact-match operation.
The source column remains unchanged, keeping the implementation modular and
safe until `DataTable` exposes integrated rows and columns.



### Brian — Grouping
