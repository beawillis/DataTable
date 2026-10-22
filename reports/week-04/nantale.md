# DataTable — Week 4 Progress Report

**Reporting period:** Week 4
**Contributor:** Nantale Swabrah (25/U/08772/PS) — Sorting and Filtering

## 1. Week 4 objective

Tested sorting in ascending and descending order for numeric and string data,
including empty results and invalid conditions. Documented the complexity and
confirmed that sorting preserves complete row relationships.

## 2. Tests completed

### 2.1 Ascending and descending order

The tests verify ascending and descending sorting for standalone columns and
table rows. Duplicate values are checked for stable ordering, so equal keys
retain their original order.

### 2.2 Numeric and string sorting

Integer, double, boolean, and mixed numeric cells are compared by numeric
value rather than by their textual representation. For example, `2` sorts
before `10`, and `2.5` sorts between `2` and `10`.

String cells use normal `std::string` lexicographical ordering. Related
values in other columns move with the sorted row.

### 2.3 Empty and missing results

Sorting an empty table with a valid column leaves it empty. Sorting an empty
standalone column returns an empty column while preserving its name.

Null cells have a defined order: they appear before populated cells in
ascending order. This allows missing values to be sorted without an
unexpected failure.

### 2.4 Invalid conditions

The tests verify that sorting an invalid table column index raises
`std::out_of_range`, including attempting to sort a table with no columns.

## 3. Complexity

For `n` values in a standalone column, `sortColumn()` copies the values in
`O(n)` time and uses `std::stable_sort()` in `O(n log n)` comparisons. The
additional storage is `O(n)`.

For a table with `n` rows and `c` columns, `sortByColumn()` copies rows in
`O(n)`, sorts them in `O(n log n)` comparisons, and rebuilds the column view
in `O(nc)`. Its additional storage is `O(n)` rows, excluding the storage
owned by the copied cells.

These are asymptotic bounds; actual runtime depends on cell types, row width,
and the build environment.

## 4. Implementation comments

- Sorting copies a standalone column before ordering it, so the source is
  unchanged.
- Table sorting reorders complete rows rather than independent columns.
- Stable sorting preserves the original order of equal keys.
- Column validation occurs before table data is modified.

## 5. OOP and integration design

The sorting module uses the shared model:

- `DataTable` supplies rows and validates column selection.
- `Row` keeps all values belonging to one record together.
- `Column` supports standalone sorted copies.
- `Cell` supplies typed values for comparison.

The implementation demonstrates encapsulation through public table and model
interfaces, composition through rows and columns containing cells, and
abstraction through `sortColumn()` and `sortByColumn()`. Inheritance and
runtime polymorphism are not required.

## 6. Validation

```text
Sort tests: passed
Ascending and descending tests: passed
Numeric and string tests: passed
Empty and invalid-condition tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Limitations and next steps

- Sorting uses the library's built-in comparison rules; no custom comparator
  API is exposed.
- Mixed non-numeric cell types use the deterministic variant ordering already
  defined by the implementation.
- Preserving row integrity and the documented complexity when adding future
  comparator features.
