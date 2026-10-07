#include "datatable/column.hpp"

#include <stdexcept>
#include <utility>

namespace datatable {

// Create an empty column after validating its required name.
Column::Column(std::string name)
    : name_{validateName(std::move(name))},
      cells_{}
{
}

// Take ownership of the supplied cells to initialize the column.
Column::Column(std::string name, std::vector<Cell> cells)
    : name_{validateName(std::move(name))},
      cells_{std::move(cells)}
{
}

// Copy the brace-enclosed cells into the column's owned storage.
Column::Column(std::string name, std::initializer_list<Cell> cells)
    : name_{validateName(std::move(name))},
      cells_{cells}
{
}

// Return the validated column name and read-only column details.
const std::string& Column::name() const noexcept
{
    return name_;
}

std::size_t Column::size() const noexcept
{
    return cells_.size();
}

bool Column::empty() const noexcept
{
    return cells_.empty();
}

const std::vector<Cell>& Column::cells() const noexcept
{
    return cells_;
}

// Use vector::at for checked access; invalid indices throw std::out_of_range.
Cell& Column::at(std::size_t index)
{
    return cells_.at(index);
}

const Cell& Column::at(std::size_t index) const
{
    return cells_.at(index);
}

// Keep bracket access checked by forwarding it to at().
Cell& Column::operator[](std::size_t index)
{
    return at(index);
}

const Cell& Column::operator[](std::size_t index) const
{
    return at(index);
}

void Column::append(Cell cell)
{
    cells_.push_back(std::move(cell)); // Move the supplied value into the column.
}

void Column::clear() noexcept
{
    cells_.clear();
}

std::string Column::validateName(std::string name)
{
    // Match DataTable's existing rule that column names cannot be empty.
    if (name.empty()) {
        throw std::invalid_argument("Column name cannot be empty.");
    }
    return name;
}

} // namespace datatable