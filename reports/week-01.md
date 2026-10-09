# DataTable — Week 1 Progress Report

**Reporting period:** Week 1
**Project:** C++ Data Table Formatting and Presentation Library
**Report owner:** Elisha — Table-Core Engineer

## 1. Week 1 objective

Establish the initial project foundation and define a small, stable `DataTable`
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
[`include/datatable/table.hpp`](../include/datatable/table.hpp) and
[`src/table.cpp`](../src/table.cpp):

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
[`tests/test_table.cpp`](../tests/test_table.cpp), covering:

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




### Primah Mukhaye — Data Model

- **Completed work:** Implemented the foundational `Cell`, `Row`, and `Column`
  data-model types. `Cell` stores null, boolean, signed and unsigned integer,
  floating-point, or string values, with typed inspection/access and text
  formatting. `Row` owns an ordered sequence of cells; `Column` owns an
  ordered sequence of cells and validates its required name. Both provide
  size/empty queries, checked indexed access, append, and clear operations.

- **Files changed:** [`cell.hpp`](../include/datatable/cell.hpp),
  [`cell.cpp`](../src/cell.cpp), [`row.hpp`](../include/datatable/row.hpp),
  [`row.cpp`](../src/row.cpp), [`column.hpp`](../include/datatable/column.hpp),
  [`column.cpp`](../src/column.cpp), and focused tests in
  [`test_cell.cpp`](../tests/test_cell.cpp),
  [`test_row.cpp`](../tests/test_row.cpp), and
  [`test_column.cpp`](../tests/test_column.cpp).

- **Tests and validation:** Added focused coverage for supported cell types,
  null and string handling, formatting and invalid typed access; row and
  column construction, mutation, clearing, and bounds errors; and empty column
  names. The tests are registered as `DataTable.cell`, `DataTable.row`, and
  `DataTable.column` in [`tests/CMakeLists.txt`](../tests/CMakeLists.txt).
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




### Manige Daniel's work completed

#### 2.1 CSV parsing foundation

- Investigated and implemented the CSV input layer for the DataTable project.
- Added the public CSV interface in [`include/datatable/csv.hpp`](../include/datatable/csv.hpp).
- Implemented the parsing logic in [`src/csv.cpp`](../src/csv.cpp).
- Supported the required CSV behaviors for:
  - plain records and empty fields
  - quoted values containing commas
  - embedded newline characters inside quoted fields
  - escaped quotes using double quotes
  - trailing delimiters and empty final fields
  - rejection of malformed quoting with `std::invalid_argument`

#### 2.2 Data ingestion tests

Added the CSV parser regression coverage in [`tests/test_csv.cpp`](../tests/test_csv.cpp), including:

- parsing standard comma-delimited output
- preserving empty values and blank records
- handling quoted fields and embedded line breaks
- validating stream input and empty-input behavior
- rejecting malformed CSV input consistently

#### 2.3 Validation and integration notes

- Verified the CSV parser through direct compile-and-run checks.
- Confirmed the parser integrates cleanly with the current project layout and test infrastructure.
- The parser currently handles raw CSV extraction only; the next step is to define how the parsed data becomes typed `Cell`, `Column`, and `Row` objects in the shared data model.

**Open decisions or blockers:**

- Agree on the conversion rules from CSV records into typed table data.
- Confirm the row-width and type-conversion rules before integrating CSV output with the main `DataTable` interface.

#### Class structure and implementation

The CSV module is intentionally a small functional interface rather than a
second table class.

- **Types used:** `CsvRecord` is a public alias for
  `std::vector<std::string>`, and `CsvData` is a public alias for
  `std::vector<CsvRecord>`. These types represent parsed records and fields
  before typed table integration.
- **Member functions:** `parseCsv(std::istream&)` reads a stream and returns
  `CsvData`; `parseCsv(const std::string&)` creates a stream from a string and
  delegates to the stream overload.
- **Data members and visibility:** there is no persistent CSV object or
  module-specific class data member. During parsing, the implementation keeps
  local state such as the current record, field, quote state, and
  `record_started` flag. Those locals are private to the parsing function,
  which prevents one parse operation from leaking state into another.
- **How it is used:** callers pass CSV text or a stream and receive raw string
  records. The parser does not infer types or create a `DataTable`; later
  integration will convert records into `Cell`, `Row`, and `Column` objects
  under the agreed shared contract.

The answers are therefore: no new CSV class was needed because parsing is a
stateless operation; the returned aliases hold records and fields; the two
public overloads perform parsing; and local private state keeps quote handling
isolated while the public functions provide the API used by the rest of the
program.




### Nantale — Sorting and Filtering

- **Completed work:** Implemented non-mutating column sorting and filtering
  using the shared `Column` and `Cell` classes. Sorting supports ascending and
  descending order, and filtering supports predicates and exact value matches.
  Numeric cells are compared numerically, strings lexicographically, nulls
  before non-null values, and mixed types deterministically by variant order.
- **Files changed:** [`sort.hpp`](../include/datatable/sort.hpp),
  [`sort.cpp`](../src/sort.cpp), [`filter.hpp`](../include/datatable/filter.hpp),
  [`filter.cpp`](../src/filter.cpp), [`test_sort.cpp`](../tests/test_sort.cpp),
  [`test_filter.cpp`](../tests/test_filter.cpp), and
  [`tests/CMakeLists.txt`](../tests/CMakeLists.txt).
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

- **Completed work:** `[BRIAN: add summary when implemented]`
- **Files changed:** `[BRIAN: add links when implemented]`
- **Tests and validation:** `[BRIAN: add results when implemented]`
- **Open decisions or blockers:** `[BRIAN: add details when implemented]`


### Hamza - Aggregation

- **Completed work:** `[BRIAN: add summary when implemented]`
- **Files changed:** `[BRIAN: add links when implemented]`
- **Tests and validation:** `[BRIAN: add results when implemented]`
- **Open decisions or blockers:** `[BRIAN: add details when implemented]`



### Paul — Formatting and Styling

- **Completed work:** `[PAUL: add summary when implemented]`
- **Files changed:** `[PAUL: add links when implemented]`
- **Tests and validation:** `[PAUL: add results when implemented]`
- **Open decisions or blockers:** `[PAUL: add details when implemented]`



### Waran — Export and Demonstration

- **Completed work:** `[WARAN: add summary when implemented]`
- **Files changed:** `[WARAN: add links when implemented]`
- **Tests and validation:** `[WARAN: add results when implemented]`
- **Open decisions or blockers:** `[WARAN: add details when implemented]`



## 6. Risks and open issues

- The typed data model is not yet connected to `DataTable`; downstream modules
  should avoid assuming an interface until the shared design is agreed.
- The current row count is initialized and reported but is not yet populated
  by row insertion because row ownership and validation belong to the shared
  data-model integration.
- Duplicate-column-name policy, row-width validation, missing-value handling,
  and type-conversion rules remain to be agreed and documented.

## 7. Plan for Week 2

1. Meet with Primah and agree the shared `Cell`, `Column`, and `Row` contracts.
2. Update the central `DataTable` API only after those contracts are agreed.
3. Add the minimum table operations required to store and access typed rows.
4. Extend core tests for row/column consistency and invalid operations.
5. Run the complete CMake build and test suite after the first integration.
6. Replace the contributor placeholders above with verified team updates.

## 8. Validation status

- Initial table-core tests were added and are registered with CTest.
- CSV parser validation has been completed for the current implementation pass.

```text
CMake configure result: successful
CMake build result: successful
CTest result: 2/2 tests passed
```

## 9. AI use

### Manige Daniel
- Tool: ChatGPT
- Purpose: Reviewing CSV parsing edge cases, validating malformed-input handling, and confirming a robust API design for string-based input.
- Reason: To reduce the risk of incorrect behavior around empty fields, quoting, and embedded newlines before implementing and testing the parser.
- Verification: The recommendations were checked against the actual parser behavior and confirmed by the passing CSV test suite.