#include "datatable/column.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace datatable {
namespace {

bool isSupportedColumnType(CellType type)
{
    switch (type) {
    case CellType::boolean:
    case CellType::signed_integer:
    case CellType::unsigned_integer:
    case CellType::floating_point:
    case CellType::string:
        return true;
    case CellType::null:
    default:
        return false;
    }
}

} // namespace

// Create an empty column after validating its required name.
Column::Column(std::string name)
    : name_{validateName(std::move(name))},
      cells_{},
      type_{}
{
}

Column::Column(std::string name, CellType type)
    : name_{validateName(std::move(name))},
      cells_{},
      type_{type}
{
    if (!isSupportedColumnType(type)) {
        throw std::invalid_argument(
            "A column must declare a supported non-null value type."
        );
    }
}

// Take ownership of the supplied cells to initialize the column.
Column::Column(std::string name, std::vector<Cell> cells)
    : name_{validateName(std::move(name))},
      cells_{std::move(cells)},
      type_{inferType(cells_)}
{
}

Column::Column(std::string name, CellType type, std::vector<Cell> cells)
    : name_{validateName(std::move(name))},
      cells_{std::move(cells)},
      type_{type}
{
    if (!isSupportedColumnType(type)) {
        throw std::invalid_argument(
            "A column must declare a supported non-null value type."
        );
    }
    for (const Cell& cell : cells_) {
        validateCellType(cell);
    }
}

// Copy the brace-enclosed cells into the column's owned storage.
Column::Column(std::string name, std::initializer_list<Cell> cells)
    : Column(std::move(name), std::vector<Cell>{cells})
{
}

// Return the validated column name and read-only column details.
const std::string& Column::name() const noexcept
{
    return name_;
}

std::optional<CellType> Column::type() const noexcept
{
    return type_;
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
const Cell& Column::at(std::size_t index) const
{
    return cells_.at(index);
}

// Keep bracket access checked by forwarding it to at().
const Cell& Column::operator[](std::size_t index) const
{
    return at(index);
}

void Column::append(Cell cell)
{
    validateCellType(cell);
    cells_.push_back(std::move(cell));
    if (!type_ && !cells_.back().isNull()) {
        type_ = cells_.back().type();
    }
}

void Column::set(std::size_t index, Cell cell)
{
    Cell& current = cells_.at(index);
    validateCellType(cell);
    current = std::move(cell);
    if (!type_ && !current.isNull()) {
        type_ = current.type();
    }
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

std::optional<CellType> Column::inferType(const std::vector<Cell>& cells)
{
    std::optional<CellType> inferred;
    for (const Cell& cell : cells) {
        if (cell.isNull()) {
            continue;
        }
        if (inferred && *inferred != cell.type()) {
            throw std::invalid_argument(
                "All non-null cells in a column must have the same type."
            );
        }
        inferred = cell.type();
    }
    return inferred;
}

void Column::validateCellType(const Cell& cell) const
{
    if (!cell.isNull() && type_ && *type_ != cell.type()) {
        throw std::invalid_argument(
            "Cell type does not match the column's declared type."
        );
    }
}

void Column::reserve(std::size_t capacity)
{
    if (cells_.capacity() < capacity) {
        const std::size_t doubledCapacity = cells_.capacity() == 0
            ? 1
            : (cells_.capacity() <= cells_.max_size() / 2
                ? cells_.capacity() * 2
                : cells_.max_size());
        cells_.reserve(std::max(capacity, doubledCapacity));
    }
}

} // namespace datatable