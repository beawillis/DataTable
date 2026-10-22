# DataTable — Week 4 Progress Report

**Reporting period:** Week 4
**Contributor:** Alele Brian Jesse — Grouping

## 1. Week 4 objective

Tested grouping with one group, multiple groups, duplicate keys, empty tables,
and invalid column selections. The tests confirmed counts, first-seen key
order, and non-mutating behavior.

## 2. Tests completed

### 2.1 Single group

A column containing only the same key produces one `Group`. Its count equals
the number of source values, and the key is retained correctly.

### 2.2 Multiple groups

Columns and tables containing several distinct keys produce one group per
distinct value. Groups retain the order in which their keys first appear.

### 2.3 Duplicate keys

Repeated string and numeric keys increase the existing group's count instead
of creating duplicate group objects. The original source values remain
unchanged.

### 2.4 Empty inputs

Grouping an empty standalone column and an empty table with a valid schema
returns an empty `GroupedColumn`. No placeholder group is created.

### 2.5 Invalid conditions

Grouping with an invalid `DataTable` column index raises
`std::out_of_range` before any grouping result is produced.

## 3. Implementation comments

- Grouping scans values in source order so first-seen order is predictable.
- Existing keys are counted through the private `Group::addValue()` helper.
- New groups store the first encountered `Cell` as their key.
- The source column or table is read only and is never modified.

## 4. Complexity

For `n` input values and `g` distinct groups, the current implementation uses
a linear search through existing groups for each value. Its time complexity is
`O(n * g)` and its result storage is `O(g)`.

In the worst case, when every value is distinct, this becomes `O(n²)`.
The implementation preserves first-seen order and keeps the code simple. A
future hash-based implementation could improve average lookup time, but would
need an explicit hashing and ordering design.

## 5. OOP and integration design

`Group` encapsulates a key and count. Only the grouping function can increment
the count, while callers receive read-only key and count access.

Grouping uses composition through `DataTable`, `Column`, and `Cell` objects.
The public `groupBy()` functions abstract the grouping process without
exposing the result's internal mutation rules. Inheritance and runtime
polymorphism are not required.

## 6. Validation

```text
Single-group tests: passed
Multiple-group tests: passed
Duplicate-key tests: passed
Empty-input tests: passed
Invalid-index tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Limitations and next steps

- Grouping currently returns keys and counts rather than grouped subtables.
- The current linear search can be improved for very high-cardinality data.
- Group-level aggregation is not yet represented in `Group`.
