#include "datatable/aggregate.hpp"

#include <stdexcept>
#include <type_traits>
#include <variant>

namespace datatable {
namespace {

// Identify the Cell alternatives that can participate in numeric calculations.
bool isNumeric(const Cell& cell)
{
    return std::holds_alternative<bool>(cell.value()) ||
        std::holds_alternative<std::int64_t>(cell.value()) ||
        std::holds_alternative<std::uint64_t>(cell.value()) ||
        std::holds_alternative<double>(cell.value());
}

double numericValue(const Cell& cell)
{
    // Normalize every supported numeric alternative to double for arithmetic.
    return std::visit(
        [](const auto& value) -> double {
            using Value = std::decay_t<decltype(value)>;
            if constexpr (std::is_arithmetic_v<Value>) {
                return static_cast<double>(value);
            }
            else {
                return 0.0;
            }
        },
        cell.value());
}

void requireNumericValues(const Column& column)
{
    // Reject unsupported values early instead of silently treating them as zero.
    for (const Cell& cell : column.cells()) {
        if (!cell.isNull() && !isNumeric(cell)) {
            throw std::invalid_argument(
                "Aggregation requires numeric or null cells."
            );
        }
    }
}

} // namespace

std::size_t count(const Column& column)
{
    std::size_t nonNullCount = 0;

    // Count only populated cells; null represents a missing value.
    for (const Cell& cell : column.cells()) {
        if (!cell.isNull()) {
            ++nonNullCount;
        }
    }

    return nonNullCount;
}

double sum(const Column& column)
{
    requireNumericValues(column);

    double total = 0.0;
    // Add each non-null numeric value to the running total.
    for (const Cell& cell : column.cells()) {
        if (!cell.isNull()) {
            total += numericValue(cell);
        }
    }

    return total;
}

double mean(const Column& column)
{
    requireNumericValues(column);

    const std::size_t valueCount = count(column);
    // An empty numeric set has no defined average.
    if (valueCount == 0) {
        throw std::invalid_argument(
            "Mean requires at least one non-null numeric cell."
        );
    }

    return sum(column) / static_cast<double>(valueCount);
}

Cell min(const Column& column)
{
    requireNumericValues(column);

    const Cell* result = nullptr;
    // Keep the smallest non-null cell encountered so far.
    for (const Cell& cell : column.cells()) {
        if (!cell.isNull() &&
            (result == nullptr || numericValue(cell) < numericValue(*result))) {
            result = &cell;
        }
    }

    if (result == nullptr) {
        throw std::invalid_argument(
            "Minimum requires at least one non-null numeric cell."
        );
    }

    return *result;
}

Cell max(const Column& column)
{
    requireNumericValues(column);

    const Cell* result = nullptr;
    // Keep the largest non-null cell encountered so far.
    for (const Cell& cell : column.cells()) {
        if (!cell.isNull() &&
            (result == nullptr || numericValue(cell) > numericValue(*result))) {
            result = &cell;
        }
    }

    if (result == nullptr) {
        throw std::invalid_argument(
            "Maximum requires at least one non-null numeric cell."
        );
    }

    return *result;
}

} // namespace datatable