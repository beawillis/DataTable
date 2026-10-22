#ifndef DATATABLE_COLUMN_HPP
#define DATATABLE_COLUMN_HPP

#include <cstddef>
#include <initializer_list>
#include <optional>
#include <string>
#include <vector>

#include "datatable/cell.hpp"

namespace datatable {

// A named, owned sequence of same-typed non-null cells.
class Column
{
public:
    // Create a named empty column or initialize it with existing cells.
    explicit Column(std::string name);
    Column(std::string name, CellType type);
    Column(std::string name, std::vector<Cell> cells);
    Column(std::string name, CellType type, std::vector<Cell> cells);
    Column(std::string name, std::initializer_list<Cell> cells);

    Column(const Column&) = default;
    Column(Column&&) noexcept = default;
    Column& operator=(const Column&) = default;
    Column& operator=(Column&&) noexcept = default;

    // Inspect the column name, cell count, and contents without copying.
    const std::string& name() const noexcept;
    // An untyped empty column infers its type from its first non-null cell.
    std::optional<CellType> type() const noexcept;
    std::size_t size() const noexcept;
    bool empty() const noexcept;

    const std::vector<Cell>& cells() const noexcept;

    // Read a cell by index; access throws when the index is invalid.
    const Cell& at(std::size_t index) const;

    const Cell& operator[](std::size_t index) const;

    // Add or replace a type-compatible cell; null is valid for every type.
    void append(Cell cell);
    void set(std::size_t index, Cell cell);
    // Remove all cells while retaining the inferred or declared type.
    void clear() noexcept;

private:
    static std::string validateName(std::string name); // Reject empty column names.
    static std::optional<CellType> inferType(
        const std::vector<Cell>& cells
    );
    void validateCellType(const Cell& cell) const;
    void reserve(std::size_t capacity);

    std::string name_; // Keep the column's identifying name.
    std::vector<Cell> cells_; // Own the column's cell values.
    std::optional<CellType> type_;

    friend class DataTable;
};

} // namespace datatable

#endif // DATATABLE_COLUMN_HPP
