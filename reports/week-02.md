# DataTable — Week 2 Progress Report

**Reporting period:** Week 2
**Contributor:** Elisha — Table-Core

## 1. Week 2 objective

Connect the central `DataTable` class to the typed `Row` and `Column` data
model so the rest of the library can access real table values through one
shared public API.

## 2. Elisha's work completed

### 2.1 Table/data-model integration

Updated [`include/datatable/table.hpp`](../include/datatable/table.hpp) and
[`src/table.cpp`](../src/table.cpp) to make `DataTable` own:

- the column schema;
- typed `Column` objects;
- typed `Row` objects; and
- the current row count.

The table now supports:

- `addColumn()` creating both a schema name and its corresponding `Column`;
- `addRow()` validating row width and adding the row to the table;
- synchronized row and column storage;
- read-only access through `rows()` and `columns()`;
- indexed column access through `column(std::size_t)`;
- name-based column access through `column(const std::string&)`;
- duplicate-column rejection;
- rejection of columns added after rows exist;
- explicit errors for mismatched row widths and missing column names;
- complete clearing of schema, columns, rows, and row count.

### 2.2 Tests

Extended [`tests/test_table.cpp`](../tests/test_table.cpp) to verify:

- rows can be inserted into a defined schema;
- row counts and column values remain synchronized;
- columns can be accessed by index and name;
- duplicate names are rejected;
- mismatched row widths are rejected; and
- missing column names raise `std::out_of_range`.

## 3. Class structure and implementation

### `DataTable`

1. **What class was updated?**
   `DataTable` is the central class that coordinates the schema, rows, and
   columns used by the rest of the program.

2. **Why?**
   The other modules need one authoritative table representation. Integrating
   `Row` and `Column` here prevents CSV, sorting, grouping, formatting, and
   export code from maintaining separate copies of table data.

3. **What data does it hold?**
   Its private members are:

   - `column_names_`: the ordered schema names;
   - `columns_`: typed columns corresponding to those names;
   - `rows_`: the table's ordered row records; and
   - `row_count_`: the current number of rows.

4. **What functions operate on that data?**
   Public query functions return counts, names, rows, and columns. `addColumn()`
   extends the schema before data exists. `addRow()` validates the row width,
   stores the row, appends each cell to its matching column, and updates the
   row count. `column()` provides indexed or name-based access. `clear()`
   resets all owned table state.

5. **Why are some members private and others public?**
   The storage members are private so callers cannot directly desynchronize
   rows, columns, names, and counts. Public functions provide controlled
   operations that enforce duplicate-name, row-width, and missing-column rules.
   This keeps the table valid for every module that consumes it.

### Existing supporting classes

`DataTable` uses the existing `Row` and `Column` classes rather than replacing
them. `Row` privately owns its ordered `cells_` and exposes public size,
access, append, and clear functions. `Column` privately owns its `name_` and
`cells_`, and exposes public name, access, append, and clear functions.
`DataTable::addRow()` uses those public interfaces to keep both views of the
same table synchronized.

## 4. Design decisions and limitations

- A row must have exactly the same number of cells as the current schema.
- Column names must be non-empty and unique.
- Columns cannot be added after the first row because existing rows would
  otherwise have inconsistent widths.
- `DataTable` owns its rows and columns by value, giving the table clear
  ownership and lifetime rules.
- The table currently exposes read-only collections through `rows()` and
  `columns()`; mutation goes through `addColumn()`, `addRow()`, and `clear()`.

## 5. Validation

```text
Build: successful
DataTable tests: passed
```

## 6. Next steps

1. Update dependent modules to consume the integrated table rows and columns.
2. Coordinate with CSV, sorting/filtering, grouping, aggregation, formatting,
   and export owners before changing shared interfaces further.
3. Add table-level integration tests for those modules.