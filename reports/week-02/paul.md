# DataTable — Week 2 Progress Report

**Reporting period:** Week 2
**Contributor:** Paul — Formatting and Styling

## 1. Week 2 objective

Implement basic console formatting for the shared `DataTable` model. The
formatter must present headers and values clearly, calculate suitable column
widths, support alignment and padding, draw optional borders, and leave the
underlying data unchanged.

## 2. Paul's work completed

### 2.1 Style configuration

The [`Style`](../../include/datatable/style.hpp) class stores presentation
choices:

- left, center, or right alignment;
- whether headers are shown;
- whether borders are shown; and
- the number of spaces used as cell padding.

Style settings are separate from table data, so changing presentation does
not change any `Cell`, `Row`, or `Column` values.

### 2.2 Console formatting

Implemented [`formatTable()`](../../include/datatable/formatter.hpp) in
[`formatter.cpp`](../../src/formatter.cpp). The formatter:

1. Reads the table's columns and rows through const access.
2. Converts cells to display text with `Cell::toString()`.
3. Calculates the required width for every column from its header and values.
4. Applies the selected alignment and padding.
5. Writes optional borders and separators.
6. Returns a console-friendly string without modifying the table.

The existing `formatColumn()` function remains available for standalone
column formatting.

### 2.3 Formatting behavior

The implementation supports:

- headers enabled or hidden;
- borders enabled or hidden;
- left, center, and right alignment;
- configurable padding;
- empty tables with a defined schema;
- empty tables with no columns;
- long values without truncation; and
- null values through their empty display text.

All cells in a table row are rendered together, preserving the table's
column order and row relationships.

### 2.4 Tests

Extended [`test_formatter.cpp`](../../tests/test_formatter.cpp) to verify:

- standalone column formatting;
- headers and borders;
- table headers and multiple rows;
- width calculation for different value lengths;
- no-decoration output;
- empty tables; and
- preservation of the source table after formatting.

The style tests continue to verify default values and style setters. The
formatter and style tests are registered as `DataTable.formatter` and
`DataTable.style`.

## 3. Class structure and implementation

### `Style`

1. **What class was created?**
   `Style`, the configuration object for console presentation.
2. **Why?**
   Formatting choices should be reusable and separate from table storage.
3. **What data does it hold?**
   Private `alignment_`, `show_header_`, `show_borders_`, and `padding_`.
4. **What functions operate on that data?**
   Accessors and setters for alignment, header visibility, border visibility,
   and padding.
5. **Why are members private and public?**
   Settings remain private so they are changed through a clear API. Public
   getters and setters allow formatters and callers to use the configuration.

### `DataTable`

1. **What class is used?**
   `DataTable` is the source of the schema, rows, and values to display.
2. **Why?**
   Formatting must present the central table model rather than copy data into
   another representation.
3. **What data does it hold?**
   Private column names, columns, rows, and row count.
4. **What functions operate on that data?**
   `columnNames()`, `columns()`, `rows()`, and `column()` provide read-only
   information used to calculate widths and render cells.
5. **Why are members private and public?**
   Private storage protects table consistency. Public const access lets the
   formatter present data without mutating it.

### `Row`, `Column`, and `Cell`

`Row` provides the ordered cells for one displayed record. `Column` provides
column names and values for width calculation. `Cell::toString()` converts
typed values and nulls into display text. These classes retain private
storage, while the formatter uses their public read-only interfaces.

### Formatting functions

The formatter uses stateless public functions rather than a formatter object.
`formatTable()` renders a complete table, and `formatColumn()` renders one
column. Internal alignment, width, and border helpers remain private to
`formatter.cpp`.

## 4. Implemented object-oriented design

### Composition

Formatting uses the existing composition structure:

- `DataTable` contains `Row` and `Column` objects.
- `Row` and `Column` contain `Cell` objects.
- `Style` contains presentation settings.
- Formatter functions combine the table model and style to produce text.

### Encapsulation

The formatter accesses data through public const interfaces. It does not
modify private table, row, column, or cell storage. `Style` also protects its
settings behind getters and setters. Formatting is therefore a presentation
operation separate from data mutation.

## 5. Design decisions and limitations

- Formatting is non-mutating.
- `formatTable()` selects widths independently for each column.
- Alignment is applied uniformly through the supplied `Style`.
- Null cells render as empty text through `Cell::toString()`.
- A table with no columns returns an empty string.
- ANSI colors and advanced themes are outside this Week 2 console-formatting
  scope.

## 6. Validation

```text
Build: successful
Formatter tests: passed
Style tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Next steps

1. Coordinate with the export owner when formatted output is reused by
   console, CSV, or HTML demonstrations.
2. Add any agreed numeric-specific alignment rules in a later iteration.
3. Extend styling only after the core formatting API remains stable.
