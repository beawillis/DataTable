# DataTable — Week 2 Progress Report

**Reporting period:** Week 2
**Contributor:** Daniel — CSV / Data I/O

## 1. Week 2 objective

Implemented CSV-to-`DataTable` loading so CSV text could be parsed, converted into
typed cells, and stored in the shared table model without creating a separate
table representation.

## 2. Daniel's work completed

### 2.1 CSV parsing

The CSV parser is implemented in
[`csv.hpp`](../../include/datatable/csv.hpp) and
[`csv.cpp`](../../src/csv.cpp). It provides:

- `parseCsv(std::istream&)` for stream input;
- `parseCsv(const std::string&)` for string input;
- `CsvRecord` as one ordered CSV record;
- `CsvData` as a collection of parsed records;
- support for comma-separated fields;
- support for CRLF, CR, and LF line endings;
- preservation of empty fields and trailing empty fields;
- quoted fields containing commas;
- escaped quotes represented by doubled quotes;
- quoted fields containing embedded newlines; and
- explicit errors for malformed or unterminated quoted fields.

The parser first produces string records. It does not create a second table
type or own table rows independently.

### 2.2 CSV-to-`DataTable` loading

The loading path uses the shared
[`DataTable::fromCsv()`](../../include/datatable/table.hpp) factory. The
implementation in [`csv.cpp`](../../src/csv.cpp):

1. Parses the input with `parseCsv()`.
2. Uses the first record as the table header.
3. Creates the table schema through `DataTable::addColumn()`.
4. Converts each later field into a typed `Cell`.
5. Creates a `Row` from the converted cells.
6. Inserts each row through `DataTable::addRow()`.

The conversion rules are:

- empty fields become null cells;
- exact `true` and `false` values become boolean cells;
- signed integer values become `std::int64_t`;
- unsigned integer values that exceed the signed range become
  `std::uint64_t`;
- other valid numeric values become `double`;
- values with leading or trailing whitespace remain strings so the original
  field text is not silently changed; and
- values outside the supported integer range remain strings rather than being
  rounded or silently corrupted.

Rows with a different number of fields from the header are rejected with
`std::invalid_argument`. Empty input produces an empty table, while a
header-only input produces a table with columns and no rows.

### 2.3 Tests

CSV and table-loading behavior is covered by
[`test_csv.cpp`](../../tests/test_csv.cpp). The tests verify:

- ordinary records and empty fields;
- quoted fields containing commas;
- embedded newlines;
- escaped quotes;
- trailing delimiters;
- stream input and empty input;
- malformed quote rejection;
- typed CSV-to-table loading;
- null, boolean, integer, and floating-point conversion;
- quoted values and header-only tables;
- signed and unsigned integer boundaries;
- whitespace preservation;
- stream-based table loading; and
- inconsistent record-width rejection.

The test is registered as `DataTable.CSV`.

## 3. Class and data structure organization

### `DataTable`

1. **What class is used?**
   `DataTable` is the shared table class that receives the CSV schema and
   rows.
2. **Why?**
   CSV input must populate the same table representation used by the other
   library modules.
3. **What data does it hold?**
   Its private members include `column_names_`, `columns_`, `rows_`, and
   `row_count_`.
4. **What functions operate on that data?**
   `fromCsv()` creates a table from parsed input. `addColumn()` creates the
   schema, and `addRow()` stores each converted record while synchronizing
   row and column views.
5. **Why are members private and public?**
   Table storage remains private so CSV loading cannot leave the table with
   mismatched rows, columns, or counts. Public functions provide controlled
   schema creation, row insertion, and access.

### `Cell`

1. **What class is used?**
   `Cell` represents one typed CSV value.
2. **Why?**
   CSV fields may represent null, boolean, integer, floating-point, or string
   data.
3. **What data does it hold?**
   Its private `value_` member stores a `CellValue` variant.
4. **What functions operate on that data?**
   Constructors create the appropriate value, while `isNull()`, `holds<T>()`,
   `get<T>()`, `getIf<T>()`, and `toString()` inspect or read it.
5. **Why are members private and public?**
   The variant is private so type access is controlled by the public API and
   invalid typed access is reported explicitly.

### `Row`

1. **What class is used?**
   `Row` stores the converted cells for one CSV record.
2. **Why?**
   It provides the ordered record that `DataTable::addRow()` validates and
   stores.
3. **What data does it hold?**
   Private `cells_`, a `std::vector<Cell>`.
4. **What functions operate on that data?**
   Constructors, `size()`, `empty()`, `cells()`, `at()`, `operator[]`,
   `append()`, and `clear()`.
5. **Why are members private and public?**
   Private storage protects the row sequence, while public checked operations
   allow table-core code to inspect and insert cells safely.

### `CsvRecord` and `CsvData`

`CsvRecord` and `CsvData` are public type aliases rather than classes.
`CsvRecord` represents `std::vector<std::string>` for one parsed record, and
`CsvData` represents `std::vector<CsvRecord>` for the complete parsed input.
They keep the parser result simple before the table-loading step converts the
strings into `Cell` objects.

## 4. Implemented object-oriented design

### Composition

The CSV loading path uses composition:

- `DataTable` contains `Row` and `Column` objects.
- `Row` and `Column` contain `Cell` objects.
- The CSV loader creates those existing objects instead of defining another
  table structure.

This allows CSV data to flow from parsed records into the same model consumed
by the rest of the DataTable library.

### Encapsulation

Encapsulation is implemented by keeping storage private:

- `DataTable` hides its schema, rows, columns, and row count.
- `Row` hides its cell vector.
- `Column` hides its name and cell vector.
- `Cell` hides its variant value.

The parser's conversion helper is private to `csv.cpp` because callers only
need the public loading and parsing functions. Public APIs expose controlled
operations and report malformed CSV, invalid widths, and invalid typed access
through exceptions.

## 5. Design decisions and limitations

- The first CSV record is always treated as the table header.
- CSV parsing and table loading use the shared `DataTable` model.
- Empty fields are represented as null cells.
- Numeric inference is conservative and preserves integer boundaries.
- Whitespace is preserved rather than trimmed automatically.
- Ragged records are rejected instead of being padded or silently truncated.
- The parser does not define a separate table ownership model.

## 6. Validation

```text
Build: successful
CSV and DataTable tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Next steps

1. Coordinating with table-core and data-model owners before changing shared
   CSV or `DataTable` interfaces.
2. Extending CSV integration only when the supported data types or table API
   change.
3. Keeping parser and conversion tests synchronized with any future CSV rules.
