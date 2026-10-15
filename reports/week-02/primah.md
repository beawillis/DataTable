# DataTable — Week 2 Progress Report

**Reporting period:** Week 2
**Contributor:** Primah — Data Model

## 1. Week 2 objective

Provide the typed `Cell`, `Row`, and `Column` model classes used by the shared
`DataTable` API, with clear ownership, checked access, and predictable value
behavior.

## 2. Primah's work completed

### 2.1 Typed data-model implementation

Implemented and stabilized the core model classes in
[`cell.hpp`](../../include/datatable/cell.hpp),
[`cell.cpp`](../../src/cell.cpp),
[`row.hpp`](../../include/datatable/row.hpp),
[`row.cpp`](../../src/row.cpp),
[`column.hpp`](../../include/datatable/column.hpp), and
[`column.cpp`](../../src/column.cpp).

The module now provides:

- `Cell` with a typed `CellValue` variant for null, bool, signed integer,
  unsigned integer, double, and string values.
- Numeric construction rules that preserve signedness for integers and store
  floating-point values as `double`.
- Safe type inspection and retrieval using `holds<T>()`, `get<T>()`, and
  `getIf<T>()`.
- `toString()` formatting for all supported alternatives, including readable
  boolean output and precision-safe formatting for `double`.
- `Row` as an ordered owned sequence of cells with checked element access,
  append, and clear behavior.
- `Column` as a named owned sequence of cells with checked element access,
  append, clear behavior, and required non-empty name validation.

### 2.2 Integration readiness for `DataTable`

The data-model API is designed for direct use by table-core operations:

- `Row` and `Column` expose consistent size/access/append/clear interfaces.
- Both `operator[]` and `at()` perform bounds-checked access by forwarding to
  `std::vector::at`.
- `Column::validateName()` enforces the same non-empty naming rule used by
  table-core schema validation.
- All model objects own data by value, which gives clear lifetime and avoids
  dangling references when the table stores rows and columns.

### 2.3 Tests

Validated the model with focused tests in:

- [`test_cell.cpp`](../../tests/test_cell.cpp)
- [`test_row.cpp`](../../tests/test_row.cpp)
- [`test_column.cpp`](../../tests/test_column.cpp)

Coverage includes:

- null and typed cell construction;
- string, integer, floating-point, and boolean behavior;
- typed access success and invalid typed-access exceptions;
- row and column construction, mutation, append, and clear behavior;
- out-of-range access exceptions; and
- empty column-name rejection.

The tests are registered as `DataTable.cell`, `DataTable.row`, and
`DataTable.column`.

## 3. Class structure and implementation

### `Cell`

1. **What class was created?**  
   `Cell`, the smallest typed value unit in the table model.
2. **Why?**  
   To represent table values safely while supporting multiple value types.
3. **What data does it hold?**  
   Private member `value_` of type `CellValue` (`std::variant`).
4. **What functions operate on that data?**  
   Constructors, `isNull()`, `value()`, `holds<T>()`, `get<T>()`,
   `getIf<T>()`, and `toString()`.
5. **Why private vs public?**  
   `value_` is private so callers cannot corrupt variant state directly.
   Public methods enforce typed access behavior and explicit exceptions.

### `Row`

1. **What class was created?**  
   `Row`, an ordered collection of cells.
2. **Why?**  
   To model one logical table record and provide consistent row operations.
3. **What data does it hold?**  
   Private member `cells_` (`std::vector<Cell>`).
4. **What functions operate on that data?**  
   Constructors, `size()`, `empty()`, `cells()`, `at()`, `operator[]`,
   `append()`, and `clear()`.
5. **Why private vs public?**  
   `cells_` is private so access goes through checked APIs and mutation rules.

### `Column`

1. **What class was created?**  
   `Column`, a named ordered collection of cells.
2. **Why?**  
   To support schema-aware column operations and shared table integration.
3. **What data does it hold?**  
   Private members `name_` and `cells_`.
4. **What functions operate on that data?**  
   Constructors, `name()`, `size()`, `empty()`, `cells()`, `at()`,
   `operator[]`, `append()`, and `clear()`.
5. **Why private vs public?**  
   Private storage protects name validity and column consistency. Public
   methods expose controlled, checked usage.

## 4. Implemented object-oriented design

### Composition

`Row` and `Column` are composed of `Cell` objects (`std::vector<Cell>`), and
they provide the model layers consumed by `DataTable`. This composition gives
clear structure from value (`Cell`) to sequence (`Row`/`Column`) to table.

### Encapsulation

Encapsulation is implemented in all three classes:

- `Cell` hides `value_`.
- `Row` hides `cells_`.
- `Column` hides `name_` and `cells_`.

Public interfaces provide controlled read/write operations with explicit error
behavior for invalid access and invalid names.

## 5. Design decisions and limitations

- The model supports a fixed set of core value types through `CellValue`.
- Type safety is strict: mismatched `get<T>()` calls throw exceptions.
- Column names are required and non-empty.
- Access is bounds-checked through `at()` semantics.
- Collections are returned as const references for read-only external access.

## 6. Validation

```text
Build: successful
Data-model tests: passed
```

## 7. Next steps

1. Continue coordinating with table-core so any shared-interface changes remain
   synchronized.
2. Add more table-level integration checks where model behavior affects
   sorting/filtering/grouping and formatting.
3. Extend test coverage as new value-handling requirements are introduced.
