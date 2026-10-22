# DataTable — Week 3 Progress Report

**Reporting period:** Week 3
**Contributor:** Nantale Swabrah (25/U/08772/PS) — Filtering

## 1. Week 3 objective

Implemented reusable filtering for standalone columns and complete `DataTable`
rows. Filtering supports predicates and equality checks, preserves row
relationships, handles missing values, validates column selection, and leaves
the source data unchanged.

## 2. Work completed

### 2.1 Standalone column filtering

`filterColumn()` returns a new `Column` containing the cells accepted by a
predicate. It preserves the source column name and the original order of
accepted cells. The source column is not modified.

`filterEquals()` provides a convenience operation for selecting cells whose
stored `CellValue` is equal to an expected `CellValue`.

### 2.2 Complete table filtering

Added table-level filtering through:

```cpp
DataTable filterByColumn(
    const DataTable& table,
    std::size_t column_index,
    const std::function<bool(const Cell&)>& predicate
);

DataTable filterEquals(
    const DataTable& table,
    std::size_t column_index,
    const Cell& expected
);
```

The implementation:

1. Validates the selected column through `DataTable::column()`.
2. Copies the complete table schema to a new result table.
3. Evaluates the predicate against the selected cell in each row.
4. Copies the complete matching row into the result.
5. Preserves the original row order and all related column values.
6. Leaves the source table unchanged.

Copying complete rows is important. Copying only the selected column would
destroy the relationship between values belonging to the same record.

### 2.3 Missing values and invalid predicates

Equality filtering supports null cells through `Cell(nullptr)`. An empty
predicate is rejected with `std::invalid_argument` for both standalone-column
and table-level filtering instead of allowing an unclear callable failure
during iteration.

Invalid column indexes raise `std::out_of_range`.

## 3. Tests

The filtering tests verify:

- standalone equality filtering;
- predicate filtering;
- duplicate matching values;
- null-cell filtering;
- complete table-row filtering;
- preservation of values in non-selected columns;
- schema preservation;
- source-table immutability;
- invalid column-index rejection; and
- empty-predicate rejection.

The test is registered as `DataTable.filter`.

## 4. Class structure and OOP implementation

### `DataTable`

`DataTable` is the shared source and result representation. Filtering reads
its public const rows, columns, and validated column selection, then builds a
new table using the normal `addColumn()` and `addRow()` invariants.

### `Column`

`Column` owns its name and cells. Standalone filtering returns a new column
with the same name and only accepted cells.

### `Row`

`Row` owns all cells belonging to one record. Table filtering copies complete
rows so values from different columns remain synchronized.

### `Cell`

`Cell` stores the typed value used by predicates and equality comparison.
Null, numeric, boolean, and string values are compared through the existing
`CellValue` representation.

### Implemented OOP concepts

- **Encapsulation:** filtering uses public interfaces and does not access
  private vectors inside `DataTable`, `Row`, or `Column`.
- **Composition:** a filtered table is composed of copied columns, rows, and
  cells.
- **Abstraction:** callers use `filterColumn()`, `filterByColumn()`, and
  `filterEquals()` without managing the result's internal storage.

Inheritance and runtime polymorphism are not required by this filtering
implementation.

## 5. Design decisions and limitations

- Filtering is non-mutating and returns a new result.
- Matching rows retain their original order.
- Table column selection uses a zero-based index.
- Invalid indexes raise `std::out_of_range`.
- Empty predicates raise `std::invalid_argument`.
- Equality compares the stored `CellValue` type and value; numeric values with
  different alternatives are not implicitly converted for equality.
- Filtering does not currently expose a custom multi-column query language.

## 6. Validation

```text
Build: successful
Filtering tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Next steps

1. Coordinating future predicate or query-expression changes with the table-core
   owner.
2. Adding agreed multi-column filtering only if it is required by the project.
3. Keeping filtering behavior aligned with future missing-value rules.
