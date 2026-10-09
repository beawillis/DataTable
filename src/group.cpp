#include "datatable/group.hpp"

#include <algorithm>
#include <utility>

namespace datatable {

Group::Group(Cell key)
    : key_(std::move(key)),
      count_(1)
{
}

const Cell& Group::key() const noexcept
{
    return key_;
}

std::size_t Group::count() const noexcept
{
    return count_;
}

void Group::addValue() noexcept
{
    ++count_;
}

GroupedColumn groupBy(const Column& column)
{
    GroupedColumn groups;

    for (const Cell& cell : column.cells()) {
        const auto match = std::find_if(
            groups.begin(),
            groups.end(),
            [&cell](const Group& group) {
                return group.key().value() == cell.value();
            });

        if (match == groups.end()) {
            // A new group keeps the first-seen order of the source column.
            groups.emplace_back(cell);
        }
        else {
            // Matching values increase the existing group's count.
            match->addValue();
        }
    }

    return groups;
}

} // namespace datatable