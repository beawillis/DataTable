#include "datatable/sort.hpp"

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
    // Copy first so sorting never changes the caller's column.
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

} // namespace datatable