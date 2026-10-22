# DataTable — Week 1 Progress Report

**Contributor:** elisha

# DataTable — Week 1 Progress Report

**Reporting period:** Week 1
**Project:** C++ Data Table Formatting and Presentation Library
**Report owner:** Elisha — Table-Core

## 1. Week 1 objective

Established the initial project foundation and defined a small, stable `DataTable`
core that the data-model, CSV, algorithm, grouping, formatting, and export
modules can build on.



## 2. Elisha Alvin Bifandhuba's work completed

### 2.1 Project foundation

- Established the CMake-based C++17 project structure.
- Added the `DataTable` library target and public include path.
- Registered the source modules and prepared test and example subdirectories.
- Added build options for tests and examples.
- Added compiler warning flags for MSVC and non-MSVC toolchains.
- Added the initial install and package-export configuration.
- Added the initial repository and CI scaffolding.

### 2.2 Initial `DataTable` core API

Implemented the first central table interface in
[`include/datatable/table.hpp`](../../include/datatable/table.hpp) and
[`src/table.cpp`](../../src/table.cpp):

- Construction of an empty table.
- `rowCount()` and `columnCount()` queries.
- `empty()` state query.
- `clear()` to remove the current schema and table state.
- `columnNames()` access to the table schema.
- `addColumn()` for adding a named column.
- Validation that rejects empty column names with
  `std::invalid_argument`.

The core currently stores column names and the row count as the initial
architecture. Typed columns, rows, and cells will be connected through the
agreed shared interfaces in the next implementation stages.

### 2.3 Core tests

Added the initial table tests in
[`tests/test_table.cpp`](../../tests/test_table.cpp), covering:

- Construction and empty-table state.
- Adding and reading column names.
- Column-count tracking.
- Rejection of empty column names.
- Clearing the table and restoring the empty state.

### 2.4 Class structure and implementation

The table core is organized around the `DataTable` class.

- **Class created:** `DataTable`, the central public object used by the rest of
  the library.
- **Data held:** private `column_names_` stores the table schema and private
  `row_count_` stores the current number of rows.
- **Member functions:** the public constructor creates an empty table;
  `rowCount()`, `columnCount()`, and `empty()` inspect its state; `clear()`
  removes the schema and resets the row count; `columnNames()` provides
  read-only schema access; and `addColumn()` adds a validated column name.
- **Public and private members:** the query and update functions are public
  because CSV, data-model, algorithm, formatting, and export modules need to
  use them. The data members are private so callers cannot directly create an
  inconsistent schema or row count. `addColumn()` performs validation and
  reports an empty name with `std::invalid_argument`.
- **How it is used:** `DataTable` is the shared table boundary. Other modules
  are expected to use this class instead of introducing separate table
  representations. Typed `Cell`, `Row`, and `Column` ownership will be
  integrated after the shared interface is agreed.

These design choices answer the implementation questions directly: the class
exists to provide one central table abstraction; it holds schema and size
state; its member functions operate on that state; and private storage
protects invariants while public functions provide controlled access.

## 3. Week 1 design decisions

- `DataTable` is the central public table representation; other modules must
  operate on it rather than introduce a second table type.
- The public API is placed under the `datatable` namespace.
- Read-only queries are marked `const` and use `noexcept` where no failure is
  expected.
- Invalid column names are reported explicitly through
  `std::invalid_argument`; invalid input is not silently accepted.
- The initial implementation keeps ownership inside `DataTable`. The final
  ownership and lifetime rules for typed cells, rows, and columns must be
  agreed with Primah before those interfaces are integrated.

## 4. Collaboration and dependency coordination

The following shared-interface decisions remain coordination points for Week 2:

- **Primah:** agree the relationship between `DataTable`, `Column`, `Row`, and
  `Cell`, including supported value types and invalid-access behavior.
- **Daniel:** agree how CSV input creates typed columns and cells.
- **Nantale:** agree whether sort and filter operations mutate the table or
  return a result.
- **Brian:** confirm the numeric-access API needed by grouping and aggregation.
- **Paul:** confirm the read-only table access needed by formatting and styling.
- **Waran:** confirm the stable public API needed by export and examples.

No shared interface should be changed silently. Any required change to
`table.hpp`, `column.hpp`, `cell.hpp`, or `row.hpp` should be discussed with
the relevant owner and Elisha first.



