# DataTable — Week 4 Progress Report

**Reporting period:** Week 4
**Contributor:** Bifandhuba Elisha Alvin — Table-Core and Integration

## 1. Week 4 objective

Completed the table-level integration, verified the project from a clean build,
exercised the public API across modules, measured a representative processing
pipeline, prepared the release installation, and documented the final result.

## 2. Work completed

### 2.1 Full public-API integration test

Added an end-to-end table-core test that performs the complete workflow:

1. Load typed records with `DataTable::fromCsv()`.
2. Filter rows by department.
3. Sort the filtered table by score.
4. Group the selected rows.
5. Aggregate the score column.
6. Format the result for console output.
7. Export the result to CSV.
8. Export the result to HTML.

The test also confirms that sorting a working copy does not modify the
filtered source table.

### 2.2 Integration demonstrations

Added two built examples:

- `integration_example` demonstrates CSV loading, filtering, sorting,
  formatting, and aggregation through the public API.
- `performance_example` processes 10,000 rows through filtering, sorting,
  grouping, and aggregation and reports the elapsed pipeline time.

### 2.3 Clean-build and release CI

Updated the GitHub Actions workflow to:

- configure a fresh `build-ci` directory;
- build the complete project in Release mode;
- run the complete CTest suite; and
- install the generated library and CMake package into `install-ci`.

This verifies that the repository can be built and installed without relying
on a developer's existing local build directory.

## 3. Performance testing

The performance example uses a steady clock around the integrated processing
pipeline. It reports:

- rows processed;
- rows selected;
- group count;
- average score; and
- elapsed pipeline time in milliseconds.

The measurement is a repeatable smoke test for integration behavior, not a
formal benchmark or a platform-independent performance claim. Results depend
on compiler, build configuration, hardware, and operating-system load.

## 4. Release preparation

The project already provides:

- C++17 configuration;
- Release-compatible CMake targets;
- public header installation;
- static library installation; and
- exported `DataTable::DataTable` package targets.

The CI install step now verifies this release path on every supported
workflow run.

## 5. Tests and validation

The table test now covers:

- CSV-to-table loading;
- filtering;
- sorting;
- grouping;
- aggregation;
- formatting;
- CSV export;
- HTML export; and
- preservation of the source table.

Validation performed locally:

```text
Clean Release configure: successful
Release build: successful
Full CTest suite: 12/12 tests passed
Integration example: builds successfully
Performance example: builds successfully
Install step: successful
```

## 6. OOP and table ownership

Elisha's Week 4 responsibility remains the central `DataTable` integration.
The table owns the shared schema, rows, and columns, while module functions
operate through its public interfaces.

The implemented design uses:

- **Encapsulation:** table state remains private and invariants are enforced
  by `addColumn()`, `addRow()`, `replaceRows()`, and validated lookup.
- **Composition:** `DataTable` contains `Row` and `Column` objects, which
  contain `Cell` objects.
- **Abstraction:** callers compose high-level operations without managing
  synchronization between row and column views.

Inheritance and runtime polymorphism are not required by the current table
architecture.

## 7. Limitations

- The performance example is a smoke test, not a full benchmark suite.
- No formal memory profiler or cross-platform performance comparison is
  included.
- The existing HTML and CSV exporters still use output streams rather than
  file-path convenience APIs.
- Advanced joins, grouped aggregate result tables, and query planning remain
  outside the current release.

## 8. Next steps

1. Running the CI workflow on the final submission branch.
2. Recording the CI result and the performance smoke-test output in the final
   presentation.
3. Keeping shared table-interface changes coordinated with all module owners.
