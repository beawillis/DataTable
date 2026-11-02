# DataTable — Week 3 Progress Report

**Contributor:** Waran — Export and Demonstration

## Export and native presentation

The export surface uses the shared `DataTable` model and writes CSV headers
and complete rows with correct quoting for commas, quotes, and line breaks.
Stream failures are reported with `std::ios_base::failure`.

The project's presentation workflow uses the optional native Qt Widgets
application. It opens CSV files in a desktop table and applies per-column
numeric value colors. The Qt target is separate from the core library and is
enabled through CMake when a compatible Qt kit is installed.

## Validation and tests

The export tests cover escaped CSV headers and values, complete records, null
fields, empty tables, and failed output streams. The Qt example is built
against the shared `DataTable` model.

## Design

- CSV export is non-mutating and preserves row order.
- Null values are written as empty CSV fields.
- Fields are quoted only when required by CSV syntax.
- The Qt viewer owns the visual presentation; the core table model remains
  independent of Qt.

## Next steps

1. Keep the Qt example aligned with changes to the public `DataTable` API.
2. Extend the native viewer to support additional interactive table workflows.
