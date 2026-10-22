# DataTable — Final Contribution Report

**Contributor:** Nantale Swabrah (25/U/08772/PS) — Sorting and Filtering

## 1. Contribution summary

Nantale implemented reusable sorting and filtering operations for the shared
`DataTable` model.

Sorting supports:

- standalone column sorting;
- table-row sorting by zero-based column index;
- ascending and descending order;
- numeric comparison;
- string comparison;
- null ordering;
- stable duplicate handling; and
- preservation of complete row relationships.

Filtering supports:

- standalone column predicates;
- standalone equality filtering;
- table predicates by selected column;
- table equality filtering;
- null-cell matching;
- source-table preservation; and
- invalid index and empty-predicate validation.

## 2. Sorting design

`sortColumn()` returns a sorted copy and leaves its source unchanged.
`sortByColumn()` copies and stably sorts complete `Row` objects, then uses
`DataTable::replaceRows()` to rebuild synchronized columns.

This design prevents values from different columns becoming detached when a
sort changes row order.

Numeric alternatives are compared by numeric value, strings use lexical
ordering, and null cells sort before populated cells in ascending order.

## 3. Filtering design

Filtering returns independent results. Table filtering copies the schema and
complete matching rows, preserving source order and the association between
all values in a record.

Invalid column indexes raise `std::out_of_range`, while empty predicates raise
`std::invalid_argument`. Equality uses the existing `CellValue` comparison,
including explicit null values.

## 4. Complexity

For `n` standalone values, sorting requires `O(n log n)` comparisons and
`O(n)` additional storage.

For `n` table rows and `c` columns, table sorting requires `O(n log n)` sort
comparisons plus `O(nc)` to rebuild synchronized columns, with `O(n)`
additional row storage.

Filtering scans rows once, requiring `O(n)` predicate evaluations. Building a
filtered table copies the selected rows and schema, so its total work is
linear in the input rows and copied cells.

## 5. Testing completed

The sorting and filtering tests cover:

- ascending and descending order;
- integers, doubles, booleans, and strings;
- numeric ordering such as `2`, `10`, and `100`;
- duplicate values and stable ordering;
- complete-row integrity;
- empty tables and empty columns;
- null values;
- invalid indexes;
- predicates;
- equality matching;
- empty predicates; and
- source-table immutability.

Final validation:

```text
Sort tests: passed
Filter tests: passed
Full CTest suite: 12/12 tests passed
```

## 6. OOP concepts

- **Encapsulation:** sorting and filtering use validated public interfaces
  rather than accessing table storage directly.
- **Composition:** `DataTable` contains rows and columns, while rows and
  columns contain cells.
- **Abstraction:** callers use high-level sorting and filtering functions
  without managing row synchronization or result construction.

Inheritance and runtime polymorphism are not implemented because these
operations do not require a class hierarchy.

## 7. Brief implementation comments

- Complete rows are sorted instead of separate columns to preserve records.
- Stable sorting keeps duplicate-key rows predictable.
- Filtering is non-mutating and produces an independent table.
- Complexity is documented using rows, columns, and selected result values.

## 8. Limitations and future work

- Custom comparators and multi-column sort keys are not currently exposed.
- Filtering does not provide a query-expression language.
- Future extensions must preserve row integrity, source immutability for
  filtering, and the documented complexity.
