#include "datatable/filter.hpp"

#include <utility>
#include <vector>

namespace datatable {

Column filterColumn(const Column& column,
                    const std::function<bool(const Cell&)>& predicate)
{
    std::vector<Cell> filtered;

    // Preserve the original order while keeping accepted cells only.
    for (const Cell& cell : column.cells()) {
        if (predicate(cell)) {
            filtered.push_back(cell);
        }
    }

    return Column(column.name(), std::move(filtered));
}

Column filterEquals(const Column& column, const Cell& expected)
{
    // Reuse predicate filtering for the common equality case.
    return filterColumn(
        column,
        [&expected](const Cell& cell) {
            return cell.value() == expected.value();
        });
}

} // namespace datatable