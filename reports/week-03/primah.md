# DataTable — Week 3 Progress Report

**Reporting period:** Week 3
**Contributor:** Primah — Data Model

## 1. Week 3 objective

Improve validation, type safety, and memory behavior in the `Cell`, `Row`,
and `Column` data-model classes. The changes must protect the shared data
model used by `DataTable` and the other modules without creating a second
representation of table data.

## 2. Work completed

### 2.1 Numeric type validation in `Cell`

`Cell` stores signed integers as `std::int64_t`, unsigned integers as
`std::uint64_t`, and floating-point values as `double`. The numeric
constructor now checks conversions that could lose information:

- oversized signed integers raise `std::out_of_range`;
- oversized unsigned integers raise `std::out_of_range`; and
- floating-point values outside the representable `double` range raise
  `std::out_of_range`.

Standard integer values that fit the supported representation retain their
correct signed or unsigned type. This prevents a value from silently becoming
truncated or an infinite floating-point value after conversion.

### 2.2 Ownership and move behavior

`Cell`, `Row`, and `Column` now explicitly declare their copy and move
operations. Their move constructors and move assignments are `noexcept`.
The classes continue to own their data directly:

- `Cell` owns its `CellValue`;
- `Row` owns its vector of cells; and
- `Column` owns its name and vector of cells.

This uses RAII and standard-library containers for automatic memory
management. Moving rows or columns can transfer their owned vectors without
requiring unnecessary element copies.

### 2.3 Existing access validation

The existing data-model validation remains active:

- a null C-string cannot be stored as a text cell;
- a column name cannot be empty;
- row and column indexed access uses bounds-checked `at()`;
- unsupported typed access throws `std::bad_variant_access`; and
- `DataTable` remains responsible for validating row widths and column
  relationships.

These checks keep invalid states visible to callers instead of silently
continuing with corrupted or ambiguous data.

## 3. Tests

The data-model tests now verify:

- preservation of supported cell types;
- largest standard unsigned integer handling;
- rejection of floating-point values outside `double` range;
- invalid typed access;
- null C-string rejection;
- invalid column names;
- bounds-checked row and column access;
- `noexcept` move construction for `Row` and `Column`; and
- move transfer of owned row and column storage.

## 4. Class structure and OOP implementation

### `Cell`

`Cell` encapsulates a private `CellValue` variant. Public constructors and
inspection functions validate and expose supported values without exposing
independent storage.

### `Row`

`Row` composes and owns an ordered `std::vector<Cell>`. Its public indexed
access is bounds-checked, while its private vector cannot be replaced
directly by users.

### `Column`

`Column` composes and owns a validated name and an ordered
`std::vector<Cell>`. It validates its name and provides controlled append,
clear, and indexed-access operations.

### OOP concepts used

The implemented concepts are:

- **Encapsulation:** data is stored in private members and accessed through
  validated public functions.
- **Composition:** rows and columns contain cells, and the central table
  contains rows and columns.
- **Abstraction:** callers work with `Cell`, `Row`, and `Column` operations
  instead of managing vectors and variant values directly.

Inheritance and runtime polymorphism are not used because the data model has
no subclass hierarchy or interchangeable virtual implementations.

## 5. Design decisions and limitations

- The supported cell types remain `null`, `bool`, signed integer, unsigned
  integer, `double`, and `std::string`.
- Numeric conversion is explicit and rejects values that cannot fit the
  supported representation.
- Data-model objects use value ownership and RAII rather than raw pointers.
- Copying remains available where table algorithms require independent rows or
  columns.
- Move operations are available for efficient ownership transfer.
- `DataTable` continues to enforce relationships between row widths and
  columns; `Row` and `Column` remain reusable independently.

## 6. Validation

```text
Build: successful
Cell tests: passed
Column tests: passed
Row tests: passed
Full CTest suite: 12/12 tests passed
```

## 7. Next steps

1. Keep future supported-type changes synchronized with CSV conversion,
   sorting, aggregation, and export.
2. Add agreed tests for any future decimal precision or non-finite-number
   policy.
3. Preserve the ownership and validation rules when extending the data model.
