# DataTable — Final Contribution Report

**Contributor:** Paul — Formatting and Styling

## 1. Contribution summary

Paul implemented the presentation layer for the shared DataTable model:

- standalone column formatting;
- complete table formatting;
- headers and optional footers;
- alignment, padding, and borders;
- per-column width calculation;
- optional fixed decimal precision; and
- configurable presentation styles.

Formatting returns text and does not modify the table, rows, columns, or
cells.

## 2. Supported behavior

The formatter supports left, center, and right alignment, optional headers,
optional borders, configurable padding, and footer rows. Widths account for
long names, long values, rendered decimal values, and footer labels.

Double values can be rendered with an explicitly selected decimal precision.
Other cell types retain their normal textual representation. Null cells
remain empty in the output.

Empty tables are handled safely:

- a schema-only table can render its headers and borders;
- a table with no columns produces an empty string.

## 3. Week 4 testing

Final formatter coverage includes:

- long and short names;
- long and short values;
- large signed and unsigned numbers;
- mixed columns containing numeric, boolean, string, and null cells;
- empty tables;
- schema-only tables;
- decimal precision and width alignment;
- footer width validation; and
- non-mutation of underlying table data.

The non-mutation test compares every stored cell value before and after
formatting, including the active variant type.

## 4. Complexity

For `r` rows and `c` columns:

- time complexity is `O(r * c)` plus rendered text length;
- width storage is `O(c)`;
- output storage is proportional to the formatted result.

Standalone column formatting is linear in the number of cells.

## 5. OOP and integration design

The formatter demonstrates:

- **Encapsulation:** it uses the public const interfaces of the shared model.
- **Composition:** it formats tables composed of columns, rows, and cells.
- **Abstraction:** callers select presentation behavior through `Style`
  rather than managing spacing and borders themselves.

No inheritance or runtime polymorphism was added because formatting does not
require a class hierarchy.

## 6. Brief implementation comments

- Width calculation uses the exact text that will be displayed.
- Footer values are presentation-only and are never appended as table rows.
- Decimal precision changes output text only.
- Const access and returned strings preserve the source table.

## 7. Validation and limitations

```text
Formatter tests: passed
Style tests: passed
Full CTest suite: 12/12 tests passed
git diff --check: passed
```

The current formatter emits plain text rather than terminal color or rich
markup. Future styling extensions should preserve the existing
non-mutating contract.
