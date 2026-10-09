#include "datatable/row.hpp"

#include <utility>

namespace datatable {

// Take ownership of an existing cell vector to construct the row.
Row::Row(std::vector<Cell> cells)
    : cells_{std::move(cells)}
{
}

// Copy a brace-enclosed list of cells into the row's owned vector.
Row::Row(std::initializer_list<Cell> cells)
    : cells_{cells}
{
}

// Return the number of cells currently stored in the row.
std::size_t Row::size() const noexcept
{
    return cells_.size();
}

// Check whether the row contains any cells.
bool Row::empty() const noexcept
{
    return cells_.empty();
}

// Expose the complete cell sequence as a read-only reference.
const std::vector<Cell>& Row::cells() const noexcept
{
    return cells_;
}

// Provide mutable, bounds-checked access to the requested cell.
Cell& Row::at(std::size_t index)
{
    return cells_.at(index);
}

// Provide read-only, bounds-checked access to the requested cell.
const Cell& Row::at(std::size_t index) const
{
    return cells_.at(index);
}

// Keep bracket-style access bounds-checked by forwarding to at().
Cell& Row::operator[](std::size_t index)
{
    return at(index);
}

// Keep const bracket-style access bounds-checked by forwarding to at().
const Cell& Row::operator[](std::size_t index) const
{
    return at(index);
}

// Move a new cell onto the end of the row.
void Row::append(Cell cell)
{
    cells_.push_back(std::move(cell));
}

// Remove every cell without removing the row itself.
void Row::clear() noexcept
{
    cells_.clear();
}

} // namespace datatable
