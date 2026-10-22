#include "datatable/sort.hpp"

#include "datatable/table.hpp"

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <variant>
#include <vector>

namespace datatable {
namespace {

// Convert supported numeric cell values to one type for comparison.
bool numericLess(const Cell& left, const Cell& right)
{
    const auto asLongDouble = [](const Cell& cell) {
        return std::visit(
            [](const auto& value) -> long double {
                using Value = std::decay_t<decltype(value)>;
                if constexpr (std::is_arithmetic_v<Value>) {
                    return static_cast<long double>(value);
                }
                else {
                    return 0.0L;
                }
            },
            cell.value());
    };

    return asLongDouble(left) < asLongDouble(right);
}

bool cellLess(const Cell& left, const Cell& right)
{
    // Place null cells before populated cells.
    if (left.isNull() != right.isNull()) {
        return left.isNull();
    }

    if (left.isNull()) {
        return false;
    }

    // Compare all numeric alternatives by value, even when their types differ.
    const bool leftNumeric = std::holds_alternative<bool>(left.value()) ||
        std::holds_alternative<std::int64_t>(left.value()) ||
        std::holds_alternative<std::uint64_t>(left.value()) ||
        std::holds_alternative<double>(left.value());
    const bool rightNumeric = std::holds_alternative<bool>(right.value()) ||
        std::holds_alternative<std::int64_t>(right.value()) ||
        std::holds_alternative<std::uint64_t>(right.value()) ||
        std::holds_alternative<double>(right.value());

    if (leftNumeric && rightNumeric) {
        return numericLess(left, right);
    }

    if (left.holds<std::string>() && right.holds<std::string>()) {
        return left.get<std::string>() < right.get<std::string>();
    }

    // Keep mixed types deterministic without pretending they are equivalent.
    return left.value().index() < right.value().index();
}

} // namespace

Column sortColumn(const Column& column, SortOrder order)
{
    // Copy first so sorting never changes the caller's column. stable_sort
    // keeps equal-key values in their original order.
    std::vector<Cell> sorted = column.cells();

    std::stable_sort(
        sorted.begin(),
        sorted.end(),
        [order](const Cell& left, const Cell& right) {
            if (order == SortOrder::ascending) {
                return cellLess(left, right);
            }
            return cellLess(right, left);
        });

    return Column(column.name(), std::move(sorted));
}

void sortByColumn(DataTable& table, std::size_t column_index, SortOrder order)
{
    // column() validates the requested index before any table changes occur.
    (void)table.column(column_index);

    // Sort complete rows so values from other columns stay with their record.
    std::vector<Row> sorted_rows = table.rows();
    std::stable_sort(
        sorted_rows.begin(),
        sorted_rows.end(),
        [column_index, order](const Row& left, const Row& right) {
            const Cell& left_cell = left.at(column_index);
            const Cell& right_cell = right.at(column_index);
            if (order == SortOrder::ascending) {
                return cellLess(left_cell, right_cell);
            }
            return cellLess(right_cell, left_cell);
        });

    table.replaceRows(sorted_rows);
}

} // namespace datatable