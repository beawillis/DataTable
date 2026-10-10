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
### Primah — Data Model

- **Completed work:** `[PRIMAH: add summary]`
- **Files changed:** `[PRIMAH: add links]`
- **Tests and validation:** `[PRIMAH: add results]`
- **Open decisions or blockers:** `[PRIMAH: add details]`



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


### Nantale — Sorting and Filtering

- **Completed work:** `[NANTALE: add summary]`
- **Files changed:** `[NANTALE: add links]`
- **Tests and validation:** `[NANTALE: add results]`
- **Open decisions or blockers:** `[NANTALE: add details]`



### Brian — Grouping and Aggregation

- **Completed work:** `[BRIAN: add summary]`
- **Files changed:** `[BRIAN: add links]`
- **Tests and validation:** `[BRIAN: add results]`
- **Open decisions or blockers:** `[BRIAN: add details]`



### Paul — Formatting and Styling

- **Completed work:** `[PAUL: add summary]`
- **Files changed:** `[PAUL: add links]`
- **Tests and validation:** `[PAUL: add results]`
- **Open decisions or blockers:** `[PAUL: add details]`



### Waran — Export and Demonstration

- **Completed work:** Defined the Week 1 CSV and HTML export design and added the stream-based `write_csv` and `write_html` API. Implemented CSV header serialization with field quoting, complete HTML document output with escaped headers, empty-table behavior, and output-stream failure reporting. Nonempty-table export is explicitly rejected until the central `DataTable` exposes row values.
- **Files changed:** [`EXPORT_DESIGN.md`](../EXPORT_DESIGN.md), [`export.hpp`](../include/datatable/export.hpp), [`export.cpp`](../src/export.cpp), [`test_export.cpp`](../tests/test_export.cpp), and [`tests/CMakeLists.txt`](../tests/CMakeLists.txt).
- **Tests and validation:** Added focused tests for CSV header escaping including commas, quotes, CR, and LF; HTML document structure and header escaping; tables with no columns; and failed output streams. Editor diagnostics and `git diff --check` passed. CMake Tools could not configure the project, so the export tests have not been executed.
- **Open decisions or blockers:** Complete row serialization depends on agreement and integration of a `DataTable` row/column value-access API. The current public table interface exposes column names and row count only. Example integration depends on the stable formatter, CSV, filtering, and grouping APIs.


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