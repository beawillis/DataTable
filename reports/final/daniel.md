# DataTable — Final Contribution Report

**Contributor:** Manige Daniel — CSV / Data I/O

## 1. Contribution summary

Daniel implemented the CSV input layer and integrated it with the shared
`DataTable` model. The module:

1. Parses CSV syntax into string records.
2. Converts records into typed `Cell` values and validated table rows.

This keeps CSV syntax handling separate from the central table representation
while retaining `DataTable::fromCsv()` as the public loading entry point.

## 2. Implemented functionality

### CSV parsing

The parser supports comma-separated fields, CRLF/LF/CR line endings, quoted
fields, commas inside quotes, escaped double quotes, embedded newlines, and
trailing empty fields.

It rejects unterminated quotes, unexpected quotes, and characters after a
closing quote.

### Typed table loading

`DataTable::fromCsv()` uses the first record as the schema and converts later
records into rows. It recognizes empty fields as null cells, exact booleans,
signed integers, supported unsigned integers, decimal doubles, and strings.
Whitespace is preserved rather than silently trimmed.

### Validation and errors

Rows whose field count differs from the header are rejected. Non-EOF stream
failures raise `std::ios_base::failure`; malformed syntax and invalid row
widths raise `std::invalid_argument`.

These distinctions prevent partially aligned tables and make input failures
visible to callers.

## 3. Testing completed

The CSV tests cover valid records, empty input, header-only input, missing
fields, quoted values, embedded newlines, escaped quotes, malformed input,
wrong row widths, stream failures, numeric boundaries, preserved whitespace,
and a 10,000-record CSV input.

The test is registered as `DataTable.CSV`.

Final validation:

```text
CSV tests: passed
Full CTest suite: 12/12 tests passed
```

## 4. OOP and integration design

The CSV module does not create a second persistent table model. Temporary
`CsvData` records are converted into the shared `DataTable`, whose rows,
columns, and cells are owned by the existing model classes.

The implemented concepts are:

- **Encapsulation:** parser state and conversion helpers are private to the
  CSV implementation.
- **Composition:** a loaded table is composed of rows, columns, and cells.
- **Abstraction:** callers use `parseCsv()` or `DataTable::fromCsv()` without
  managing quote state, conversion, or row validation.

Inheritance and runtime polymorphism are not used because CSV loading does not
require a class hierarchy.

## 5. Week 4 completion

Week 4 confirmed correct handling of normal, empty, missing, malformed, and
large inputs. The large-input test uses 10,000 generated records and checks
both structure and boundary values.

### Brief implementation comments

- Empty fields retain their positions before becoming null cells.
- Schema validation happens before a record is added.
- EOF is accepted, while other stream failures are reported.
- The large-input test checks correctness without making unsupported
  performance claims.

## 6. Limitations and future work

- No dedicated file-path loading API is currently exposed.
- Delimiters and CSV dialect rules are not configurable.
- The large-file test is a correctness smoke test, not a formal benchmark.
- New CSV conversion rules must stay synchronized with `CellValue` and the
  other table modules.
