# DataTable — Week 4 Progress Report

**Reporting period:** Week 4
**Contributor:** Mukhaye Primah — Cell, Row, and Column Data Model

## 1. Week 4 objective

Test the data-model classes thoroughly using empty tables and containers,
empty rows, wrong types, invalid indexes, valid mixed data, and large
collections. The tests must confirm that valid operations remain usable and
invalid access is reported explicitly.

## 2. Tests completed

### 2.1 Empty values and containers

The tests verify:

- a default `Cell` represents a valid null value;
- an explicit null cell behaves the same way;
- an empty `Row` is valid;
- an empty `Column` is valid when its name is non-empty; and
- clearing rows and columns leaves valid reusable objects.

### 2.2 Valid mixed data

The data model is tested with strings, signed and unsigned integers, floating
point values, booleans, and null cells. The tests verify that each supported
value retains its expected `CellValue` alternative and text representation.

### 2.3 Wrong types

Requesting a type different from the value stored in a `Cell` raises
`std::bad_variant_access`. The non-throwing `getIf<T>()` path is also tested:
it returns `nullptr` for a wrong type and a pointer for the matching type.

### 2.4 Invalid indexes and names

The tests verify that:

- invalid row indexes throw `std::out_of_range`;
- invalid column indexes throw `std::out_of_range`;
- empty column names throw `std::invalid_argument`; and
- null C-string input is rejected.

### 2.5 Large collections

The tests append 10,000 cells to a `Row`, 10,000 cells to a `Column`, and
construct 10,000 independent numeric cells. They verify the final size and
last value, confirming that the owning vectors remain usable as they grow.

## 3. Data-model behavior verified

`Cell` owns one typed value. `Row` owns an ordered vector of cells, and
`Column` owns a validated name and an ordered vector of cells. The tests
therefore exercise both value correctness and ownership behavior without
using raw pointers or manual deallocation.

Move construction remains `noexcept` for the data-model classes, allowing
standard containers to transfer owned storage efficiently.

## 4. OOP concepts

- **Encapsulation:** cell, row, and column storage is private and accessed
  through validated public operations.
- **Composition:** rows and columns contain cells.
- **Abstraction:** callers use `Cell`, `Row`, and `Column` APIs instead of
  managing variant values and vectors directly.

Inheritance and runtime polymorphism are not used because these model classes
do not require a subclass hierarchy.

## 5. Validation

```text
Cell tests: passed
Row tests: passed
Column tests: passed
Full CTest suite: 12/12 tests passed
```

## 6. Limitations and next steps

- The data model supports the currently agreed `CellValue` alternatives only.
- Large-data tests validate correctness and growth behavior, but are not a
  formal memory benchmark.
- Future value types must be added consistently to conversion, comparison,
  formatting, aggregation, and export code.
