# DataTable — Week 3 Progress Report

**Reporting period:** Week 3
**Contributor:** Tushabe Paul — Formatting and Styling

## 1. Week 3 objective

Extend console presentation so callers can control alignment, column widths,
headers, footers, decimal formatting, borders, and other display styles
without changing the underlying `DataTable`.

## 2. Work completed

### 2.1 Alignment and widths

`formatTable()` and `formatColumn()` support left, center, and right
alignment through `Style`. Padding and borders are configurable.

Table column widths are calculated from the longest displayed value in each
column. Header and footer text are included in the width calculation, so
formatted output remains aligned when labels are longer than data values.

### 2.2 Headers and footers

Headers are controlled by `Style::setShowHeader()`. The table formatter can
also render a footer row configured with:

```cpp
style.setFooter({"Total", "300.50"});
```

The footer must contain exactly one value per table column. An invalid footer
width raises `std::invalid_argument` instead of producing a misaligned table.
Footers are presentation-only and do not become table rows.

### 2.3 Decimal formatting

Callers can request fixed decimal precision for floating-point cells:

```cpp
style.setDecimalPrecision(2);
```

Only cells stored as `double` are reformatted. Integers, booleans, strings,
and null cells retain their normal representations. Precision can be removed
with `clearDecimalPrecision()`, returning to the cell's default text
conversion.

### 2.4 Styles and non-mutating output

`Style` stores alignment, header visibility, border visibility, padding,
decimal precision, and footer values. `formatTable()` and `formatColumn()`
read these settings and return strings without modifying rows, columns, or
cells.

## 3. Tests

The formatter and style tests verify:

- default and custom alignment;
- configurable padding and borders;
- optional headers;
- automatic width calculation;
- fixed decimal precision;
- footer rendering;
- invalid footer-width rejection;
- schema-only and empty-table formatting; and
- preservation of source table and column data.

## 4. Class structure and OOP implementation

### `Style`

`Style` encapsulates presentation settings in private members. Public
getters and setters provide controlled configuration without exposing the
formatter's internal rendering logic.

### `DataTable`, `Row`, `Column`, and `Cell`

The formatter reads the shared table composition:

- `DataTable` provides columns and rows;
- `Row` provides complete records;
- `Column` provides names and column structure; and
- `Cell` provides typed values for display conversion.

The formatter does not take ownership of or mutate these objects.

### Implemented OOP concepts

- **Encapsulation:** style settings and table data remain private behind
  public interfaces.
- **Composition:** the table is composed of rows, columns, and cells, while
  formatting uses a separate `Style` object.
- **Abstraction:** callers request formatted output without managing widths,
  borders, alignment, or numeric conversion themselves.

Inheritance and runtime polymorphism are not required by this formatting
implementation.

## 5. Design decisions and limitations

- Formatting is non-mutating.
- Widths are calculated from the displayed text.
- Decimal precision applies to stored `double` cells only.
- Footer values are presentation strings and are not added to the table.
- Footer width must match the table column count.
- Advanced themes, colors, multiline cell layout, and automatic aggregate
  footers are not implemented.

## 6. Validation

```text
Build: successful
Formatter tests: passed
Style tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Next steps

1. Coordinate any new formatting options with the table and export owners.
2. Add agreed support for multiline display values if required.
3. Consider reusable footer helpers after aggregation requirements are
   finalized.
