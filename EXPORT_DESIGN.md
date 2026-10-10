# Export and Example Design (Week 1)

Owner: Waran Newton Mono Korsuk
Status: Proposal for agreement with the table and data-model owners

## Scope

This module serializes an existing `DataTable`; it does not parse or load CSV (owned by the CSV module), mutate table data, or provide console formatting (owned by the formatter module). The first release supports CSV and HTML output for INTEGER, DOUBLE, STRING, and BOOLEAN columns.

The current `Cell`, `Row`, and `Column` types exist, but `DataTable` does not yet expose its row or column values. The export API uses the stable table schema interface for now; confirm the eventual table iteration API and shared error conventions with the architecture/data-model owners before completing data-row serialization.

## Week 1 implementation status

The stream API is implemented against the current `DataTable` schema interface. CSV and HTML output include column headers and handle empty tables, escaping, and stream failures. The current `DataTable` has no public row-value access, so export rejects a table with rows rather than silently writing incomplete data. Complete data-row serialization depends on agreement and integration of the table/data-model access API.

## Proposed public API

Use stream overloads as the core interface so callers can write to files, memory streams, or other sinks. Add filesystem convenience overloads if the agreed project API needs them.

```cpp
namespace datatable {

class DataTable;

void write_csv(const DataTable& table, std::ostream& output);
void write_html(const DataTable& table, std::ostream& output);

} // namespace datatable
```

The public header should include `<iosfwd>` and forward-declare `DataTable`. The implementation should include the complete table definition. Stream write failures should be reported as `std::ios_base::failure`; invalid table state should follow the error convention established by the core API rather than introducing a separate export-only error system.

## Serialization rules

### CSV

- Write column names as the first record, then one record per table row, preserving column and row order.
- Separate fields with commas and terminate records with CRLF.
- Quote a field if it contains a comma, quote, CR, or LF; double every quote inside a quoted field.
- Emit strings as their original UTF-8 bytes. Do not trim or otherwise change their content.
- Emit booleans as `true` or `false` in lowercase.
- Emit integer and double values as locale-independent numeric text. The numeric format must be confirmed against the agreed `Cell` representation; doubles should use enough precision for round-tripping where practical.
- Emit an empty table as a header record if it has columns; a table with no columns emits no records.
- Missing-value/null semantics are not defined by the current requirements. Do not invent a null token; agree this with the data-model owner before implementation.

### HTML

- Emit a complete UTF-8 HTML document containing one semantic `<table>`, with column names in `<thead>` and data rows in `<tbody>`.
- Escape text in both headers and cells: `&` to `&amp;`, `<` to `&lt;`, `>` to `&gt;`, `"` to `&quot;`, and `'` to `&#39;`.
- Use the same boolean and numeric textual representations as CSV.
- Do not add scripts, styles, or interpret cell contents as markup.
- An empty table still contains its header (when columns exist) and an empty `<tbody>`.

## Example plan

Examples should show library usage rather than duplicate implementation details.

- `examples/basic_table.cpp`: construct a small table and display it using the agreed formatter API.
- `examples/csv_example.cpp`: write a table to a CSV file and demonstrate a string containing a comma or quote.
- `examples/filtering_example.cpp` and `examples/grouping_example.cpp`: demonstrate those APIs only after their owners agree the interfaces; these are integration examples, not Week 1 export prerequisites.
- `examples/export_example.cpp`: write the same table to CSV and HTML, including a value that must be escaped in each format.

Each example should build as a CMake target, use only public headers, check/report output errors, and avoid hard-coded machine-specific paths. Keep the core CSV/HTML export usable without the example programs.

## Focused test checklist

- CSV headers, row/column ordering, and empty-table behavior.
- CSV escaping of comma, quote, CR, and LF; unquoted ordinary values.
- CSV output for all four supported types and locale-independent decimals.
- HTML structure and escaping for headers and cell values, including markup-like input.
- HTML output for empty tables.
- Failed output streams report errors.
- Export does not change table contents.

## Week 1 decisions to confirm

1. Exact `DataTable`/`Cell` public interfaces and how columns are iterated.
2. Whether export errors use exceptions or a project-wide result type.
3. Whether missing values exist and, if so, their CSV/HTML representation.
4. Whether CSV output uses CRLF and UTF-8 as specified above.
5. Which examples can compile in the first integrated milestone based on the timing of formatter/filter/group APIs.
