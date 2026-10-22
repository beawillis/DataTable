# DataTable — Final Contribution Report

**Contributor:** Mukhaye Primah — Cell, Row, and Column Data Model

## 1. Contribution summary

Primah implemented and maintained the foundational data-model classes used by
the shared `DataTable`:

- `Cell` represents one typed or missing value.
- `Row` owns the ordered cells belonging to one record.
- `Column` owns a validated name and ordered cells belonging to one field.

These classes provide the value ownership, type inspection, checked access,
and composition structure used by CSV loading, sorting, filtering, grouping,
aggregation, formatting, and export.

### Brief implementation comments

- `CellValue` centralizes supported types so every module uses the same value
  representation.
- Private vectors in `Row` and `Column` protect ownership while their public
  checked access keeps invalid indexes visible.
- RAII and move operations allow values to be stored and transferred without
  manual memory management.

## 2. Implemented functionality

### 2.1 Typed cells

`CellValue` supports:

- null through `std::monostate`;
- `bool`;
- `std::int64_t`;
- `std::uint64_t`;
- `double`; and
- `std::string`.

`Cell` provides constructors, `isNull()`, `holds<T>()`, `get<T>()`,
`getIf<T>()`, `value()`, and `toString()`. Typed access is explicit:
requesting an incorrect type through `get<T>()` raises
`std::bad_variant_access`, while `getIf<T>()` allows non-throwing inspection.

Numeric construction preserves integer signedness and validates values that
cannot fit in the supported representation. Floating-point values outside
the supported `double` range are rejected.

### 2.2 Rows and columns

`Row` owns a `std::vector<Cell>` and provides:

- empty and initialized construction;
- size and empty queries;
- checked `at()` and `operator[]` access;
- append;
- clear; and
- read-only sequence access.

`Column` owns a validated name and a `std::vector<Cell>`. It provides the same
sequence operations while rejecting empty names.

Both classes use value ownership and standard containers, so their lifetimes
are managed automatically through RAII.

### 2.3 Integration with `DataTable`

The data-model classes were designed as the shared foundation for Elisha’s
central `DataTable`. `DataTable` contains rows and columns, while rows and
columns contain cells:

```text
DataTable
├── Row
│   └── Cell
└── Column
    └── Cell
```

Rows and columns expose the interfaces required by table-level operations while
keeping their storage private. `DataTable` remains responsible for enforcing
relationships between its schema, row widths, and synchronized row/column
views.

## 3. Validation and safety

The implementation explicitly handles invalid operations:

- empty column names raise `std::invalid_argument`;
- null C-string values are rejected;
- invalid row and column indexes raise `std::out_of_range`;
- incorrect typed cell access raises `std::bad_variant_access`;
- numeric values that cannot fit the supported representation raise
  `std::out_of_range`; and
- `getIf<T>()` returns `nullptr` instead of throwing for a type mismatch.

The model classes also explicitly provide copy and `noexcept` move operations.
This allows table algorithms to copy independent values when required and
transfer owned vectors efficiently when moving.

These rules are important because table operations must preserve both the
value's type and the relationship between cells belonging to one row.

## 4. Testing completed

Focused data-model tests cover:

- null, boolean, integer, floating-point, and string cells;
- valid mixed rows and columns;
- text conversion;
- wrong typed access;
- non-throwing typed inspection;
- null C-string rejection;
- numeric boundary validation;
- empty rows;
- empty named columns;
- row and column clearing;
- invalid indexes;
- invalid column names;
- move construction; and
- collections containing 10,000 cells.

The tests are registered as:

- `DataTable.cell`;
- `DataTable.row`; and
- `DataTable.column`.

Final project validation completed with:

```text
Cell tests: passed
Row tests: passed
Column tests: passed
Full CTest suite: 12/12 tests passed
```

## 5. Object-oriented design

### Encapsulation

`Cell` hides its `CellValue`, `Row` hides its cell vector, and `Column` hides
its name and cell vector. Public operations enforce type, name, and index
rules rather than exposing unrestricted internal storage.

### Composition

`Row` and `Column` are composed of `Cell` objects. `DataTable` is composed of
rows and columns. This gives the project a clear structure from individual
values to complete records and fields.

### Abstraction

Callers use `Cell`, `Row`, and `Column` operations without managing variant
storage, vector ownership, or manual memory deallocation directly.

Inheritance and runtime polymorphism are not implemented because the current
data model does not require subclassing or interchangeable virtual
implementations.

## 6. Design decisions and limitations

- The supported value set is intentionally fixed through `CellValue`.
- Values are owned directly; raw pointers are not used.
- Checked access is preferred so invalid indexes are reported explicitly.
- Copying remains available for independent table results and algorithms.
- Move operations support efficient ownership transfer.
- Large-data tests verify correctness and container growth, but are not a
  formal memory or performance benchmark.
- Adding a new value type requires coordinated updates to CSV conversion,
  sorting, aggregation, formatting, and export.

## 7. Final reflection

The data-model module provides the foundation on which the rest of DataTable
depends. Keeping `Cell`, `Row`, and `Column` type-safe, ownership-aware, and
validated allows the table core and specialized modules to operate on a
consistent representation. The final implementation favors composition,
encapsulation, explicit errors, and RAII over unnecessary inheritance or
manual memory management.
