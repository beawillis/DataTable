#ifndef DATATABLE_GROUP_HPP
#define DATATABLE_GROUP_HPP

#include <cstddef>
#include <vector>

#include "datatable/cell.hpp"
#include "datatable/column.hpp"

namespace datatable {

// Stores one distinct grouping key and the number of values assigned to it.
class Group
{
public:
    explicit Group(Cell key);

    const Cell& key() const noexcept;
    std::size_t count() const noexcept;

private:
    friend std::vector<Group> groupBy(const Column& column);

    // Incremented only by groupBy when another matching value is found.
    void addValue() noexcept;

    Cell key_;
    std::size_t count_;
};

using GroupedColumn = std::vector<Group>;

// Group equal cells while preserving the order of first appearance.
GroupedColumn groupBy(const Column& column);

} // namespace datatable

#endif // DATATABLE_GROUP_HPP