#include "datatable/filter.hpp"

#include "datatable/table.hpp"

#include <stdexcept>
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

DataTable filterByColumn(
        const DataTable& table,
        std::size_t column_index,
        const std::function<bool(const Cell&)>& predicate
)
{
        // Validate the column before constructing the filtered result.
        (void)table.column(column_index);

        DataTable filtered;
        for (const Column& column : table.columns()) {
            filtered.addColumn(column.name());
        }

        // Copy complete rows so values in other columns stay associated.
        for (const Row& row : table.rows()) {
            if (predicate(row.at(column_index))) {
                filtered.addRow(row);
            }
        }

        return filtered;
}

DataTable filterEquals(
        const DataTable& table,
        std::size_t column_index,
        const Cell& expected
)
{
        return filterByColumn(
            table,
            column_index,
            [&expected](const Cell& cell) {
                return cell.value() == expected.value();
            }
        );
}

} // namespace datatable