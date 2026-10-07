#ifndef DATATABLE_ROW_HPP
#define DATATABLE_ROW_HPP

#include <cstddef>
#include <initializer_list>
#include <vector>

#include "datatable/cell.hpp"

namespace datatable {

// An ordered, owned sequence of cells representing one table row.
class Row
{
public:
    // Create an empty row or initialize it with existing cells.
    Row() = default;
    Row(std::vector<Cell> cells);
    Row(std::initializer_list<Cell> cells);

    // Inspect the number of cells and read the complete sequence without copying.
    std::size_t size() const noexcept;
    bool empty() const noexcept;

    const std::vector<Cell>& cells() const noexcept;

    // Access a cell by index; both access forms throw when the index is invalid.
    Cell& at(std::size_t index);
    const Cell& at(std::size_t index) const;

    Cell& operator[](std::size_t index);
    const Cell& operator[](std::size_t index) const;

    // Add a cell to the end or remove every cell.
    void append(Cell cell);
    void clear() noexcept;

private:
    std::vector<Cell> cells_; // The row owns its cells and their values.
};

} // namespace datatable

#endif // DATATABLE_ROW_HPP
