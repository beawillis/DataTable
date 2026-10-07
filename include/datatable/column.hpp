#ifndef DATATABLE_COLUMN_HPP
#define DATATABLE_COLUMN_HPP

#include <cstddef>
#include <initializer_list>
#include <string>
#include <vector>

#include "datatable/cell.hpp"

namespace datatable {

// A named, owned sequence of cells representing one table column.
class Column
{
public:
    // Create a named empty column or initialize it with existing cells.
    explicit Column(std::string name);
    Column(std::string name, std::vector<Cell> cells);
    Column(std::string name, std::initializer_list<Cell> cells);

    // Inspect the column name, cell count, and contents without copying.
    const std::string& name() const noexcept;
    std::size_t size() const noexcept;
    bool empty() const noexcept;

    const std::vector<Cell>& cells() const noexcept;

    // Access a cell by index; both access forms throw when the index is invalid.
    Cell& at(std::size_t index);
    const Cell& at(std::size_t index) const;

    Cell& operator[](std::size_t index);
    const Cell& operator[](std::size_t index) const;

    // Add a cell to the end or remove all cells while preserving the name.
    void append(Cell cell);
    void clear() noexcept;

private:
    static std::string validateName(std::string name); // Reject empty column names.

    std::string name_; // Keep the column's identifying name.
    std::vector<Cell> cells_; // Own the column's cell values.
};

} // namespace datatable

#endif // DATATABLE_COLUMN_HPP
