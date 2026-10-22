# DataTable — Final Contribution Report

**Contributor:** Alele Brian Jesse — Grouping

## 1. Contribution summary

Brian implemented grouping for standalone columns and shared `DataTable`
columns. The module returns `Group` objects containing each distinct key and
the number of source values assigned to that key.

Supported behavior includes:

- single groups;
- multiple groups;
- duplicate key counting;
- string, numeric, boolean, and null cell keys through `CellValue`
  comparison;
- first-seen key ordering;
- empty inputs; and
- invalid table-column validation.

## 2. Grouping design

`Group` stores its key and count privately. Its public interface provides
read-only `key()` and `count()` access. The private `addValue()` operation is
available only to the grouping implementation, so callers cannot change a
group count arbitrarily.

`groupBy(const Column&)` reads the source column and returns a new
`GroupedColumn`. `groupBy(const DataTable&, index)` validates the selected
column and delegates to the column-level operation.

Grouping is non-mutating. The source table and column remain unchanged.

## 3. Complexity

With `n` input values and `g` distinct keys, grouping currently performs
`O(n * g)` comparisons and stores `O(g)` result groups. In the worst case,
where every input value is distinct, the time complexity is `O(n²)`.

This implementation preserves first-seen order directly. A future hash-based
approach could provide faster average lookup, but it would need to preserve
the same ordering and support all accepted `CellValue` alternatives.

## 4. Testing completed

The grouping tests cover:

- one repeated key producing one group;
- multiple distinct keys;
- duplicate string keys;
- duplicate numeric keys;
- first-seen group ordering;
- group counts;
- empty standalone columns;
- empty tables with a schema;
- source-table preservation; and
- invalid column indexes.

Final validation:

```text
Group tests: passed
Full CTest suite: 12/12 tests passed
```

## 5. OOP concepts

- **Encapsulation:** `Group` hides its key and mutable count.
- **Composition:** grouping reads `DataTable`, `Column`, and `Cell` objects.
- **Abstraction:** callers use `groupBy()` without managing group creation or
  duplicate-key counting.

Inheritance and runtime polymorphism are not implemented because grouping does
not require a class hierarchy.

## 6. Brief implementation comments

- First-seen order is preserved by scanning the source sequentially.
- Duplicate keys update an existing group instead of creating another group.
- Empty input returns no groups rather than a synthetic empty key.
- Invalid indexes are rejected through the central table API.

## 7. Limitations and future work

- Grouped results contain counts but not grouped row collections.
- Group-level aggregation is not currently part of `Group`.
- High-cardinality inputs may benefit from an ordered hash/index structure in
  a future optimization.
