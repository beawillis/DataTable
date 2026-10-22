# DataTable — Week 2 Progress Report

**Reporting period:** Week 2
**Contributor:** Brian — Grouping

## 1. Week 2 objective

Implemented grouping functionality that can identify distinct values in a
selected table column, count how many rows belong to each group, and preserve
the original table without changing its row or column data.

## 2. Brian's work completed

### 2.1 Grouping data structures

The grouping module uses the `Group` class in
[`group.hpp`](../../include/datatable/group.hpp) and
[`group.cpp`](../../src/group.cpp).

Each `Group` stores:

- one distinct `Cell` key; and
- the number of source values that match that key.

`GroupedColumn` is a vector of `Group` objects. Groups are created in the
order their keys first appear in the source data, so grouping is deterministic
and does not unexpectedly reorder the input.

### 2.2 Table-level grouping

Added a table-level overload:

```cpp
GroupedColumn groupBy(
    const DataTable& table,
    std::size_t column_index
);
```

The implementation:

1. Validates the requested column index through `DataTable::column()`.
2. Selects the requested `Column`.
3. Reuses the existing column grouping algorithm.
4. Returns the distinct keys and their counts.

The table is passed as `const`, so grouping cannot modify its rows, columns,
schema, or row count.

### 2.3 Grouping behavior

Grouping:

- counts repeated values;
- preserves first-seen key order;
- supports the existing `Cell` value representation;
- returns an empty result for an empty selected column;
- leaves the original table unchanged; and
- reports an invalid column index with `std::out_of_range`.

The existing `groupBy(const Column&)` API remains available for callers that
already have a standalone column.

### 2.4 Tests

Extended [`test_group.cpp`](../../tests/test_group.cpp) to verify:

- grouping a standalone column;
- repeated key counts;
- first-seen group order;
- grouping a selected `DataTable` column;
- preservation of related values in other columns;
- empty-table grouping; and
- invalid column-index rejection.

The test is registered as `DataTable.group`.

## 3. Class structure and implementation

### `Group`

1. **What class was created?**
   `Group`, the result object for one distinct grouping key.
2. **Why?**
   To keep a key and its occurrence count together.
3. **What data does it hold?**
   Private `key_` of type `Cell` and private `count_` of type `std::size_t`.
4. **What functions operate on that data?**
   The public constructor, `key()`, and `count()` provide result access.
   The private `addValue()` function increments a count during grouping.
5. **Why are members private and public?**
   The key and count are private so callers cannot change grouping results
   directly. Public accessors expose the completed result, while the internal
   increment operation is restricted to the grouping implementation.

### `DataTable`

1. **What class is used?**
   `DataTable` is the central source of rows and columns.
2. **Why?**
   Grouping must operate on the shared table model rather than a separate
   table representation.
3. **What data does it hold?**
   Private schema, row, column, and row-count members.
4. **What functions operate on that data?**
   `column(std::size_t)` selects the grouping column, while `rows()` and
   `columns()` allow the tests and other modules to inspect the unchanged
   table.
5. **Why are members private and public?**
   Private storage protects table consistency. The grouping API receives a
   const table and uses public accessors without modifying the source.

### `Cell` and `Column`

`Cell` stores each grouping key in its private `CellValue` variant. `Column`
stores the ordered source values in its private `name_` and `cells_` members.
The grouping implementation uses their public `value()`, `cells()`, `key()`,
and `count()` interfaces instead of accessing storage directly.

## 4. Implemented object-oriented design

### Composition

Grouping uses the existing composition structure:

- `DataTable` contains `Column` objects.
- A `Column` contains `Cell` objects.
- A `Group` owns a `Cell` key.
- `GroupedColumn` contains the resulting `Group` objects.

### Encapsulation

`Group` keeps its key and count private and exposes read-only result access.
`DataTable`, `Column`, and `Cell` also keep their storage private. The
grouping implementation uses public APIs and returns a separate result, so the
source table cannot be corrupted by grouping.

## 5. Design decisions and limitations

- Grouping is non-mutating.
- Group order follows first appearance in the selected column.
- Column selection uses a zero-based column index.
- Invalid column indices raise `std::out_of_range`.
- Equality follows the existing `CellValue` variant equality behavior.
- Aggregation operations such as sum and mean remain separate from this
  grouping module.

## 6. Validation

```text
Build: successful
Grouping tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Next steps

1. Coordinating with the aggregation owner when grouped results are combined
   with count, sum, mean, minimum, or maximum operations.
2. Adding any agreed mixed-type or null-grouping rules to the tests.
3. Keeping grouping table-level APIs aligned with future DataTable changes.
