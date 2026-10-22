# DataTable — Week 4 Progress Report

**Contributor:** Paul — Formatting and Styling

## 1. Week 4 objective

Validated that formatting handles long and short names, large numeric values,
empty tables, and mixed column types without changing the underlying data.

## 2. Tests completed

The formatter tests now cover:

- short and deliberately long column names;
- short and long text values;
- signed and unsigned large integers;
- boolean, string, null, and numeric cells in the same table;
- schema-only tables with no rows;
- completely empty tables with no columns; and
- preservation of row values, cell types, row count, and column names after
  formatting.

The tests also retain the existing coverage for headers, borders, alignment,
padding, decimal precision, footers, and invalid footer widths.

## 3. Comments and implementation analysis

- Widths are calculated independently for each column using headers, rendered
  values, and footer text.
- Formatted decimal text is used for width calculation, so displayed values
  remain aligned.
- Null cells render as empty content rather than being converted into data.
- The formatter reads rows and columns through const interfaces and returns a
  string; it does not write to the `DataTable`.
- Style settings are presentation state and are separate from stored cells.

## 4. Complexity

For `r` rows and `c` columns, table formatting scans the table values once:

- width calculation: `O(r * c)`;
- output generation: `O(r * c)`, plus the length of rendered text;
- auxiliary width storage: `O(c)`.

Formatting a standalone column with `n` cells is `O(n)` for width discovery
and output generation.

## 5. OOP and integration design

The module uses the shared `DataTable`, `Column`, `Row`, and `Cell` model
without introducing a second representation. Encapsulation is preserved
through public read-only accessors, composition is represented by tables
containing columns and rows containing cells, and `Style` abstracts
presentation choices from stored data. Inheritance and runtime polymorphism
are not used.

## 6. Validation

```text
Formatter tests: passed
Style tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Limitations and next steps

- Output is currently a plain-text table representation.
- Extremely large rendered strings require corresponding memory for the
  returned output.
- Preserving the non-mutating formatting contract when adding future styling
  features.
