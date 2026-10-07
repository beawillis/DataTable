#include "datatable/cell.hpp"

#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace datatable {

// nullptr and default construction both represent a null cell.
Cell::Cell(std::nullptr_t) noexcept
    : value_{}
{
}

Cell::Cell(bool value) noexcept
    : value_{value}
{
}

Cell::Cell(std::string value)
    : value_{std::move(value)}
{
}

Cell::Cell(const char* value)
    : value_{}
{
    if (value == nullptr) {
        throw std::invalid_argument("Cell string value cannot be null.");
    }
    value_ = std::string(value); // Copy the C string into owned storage.
}

bool Cell::isNull() const noexcept
{
    return holds<std::monostate>();
}

const CellValue& Cell::value() const noexcept
{
    return value_;
}

CellValue& Cell::value() noexcept
{
    return value_;
}

std::string Cell::toString() const
{
    // Visit the active alternative so each supported type gets suitable formatting.
    return std::visit(
        [](const auto& value) -> std::string {
            using ValueType = typename std::decay<decltype(value)>::type;

            if constexpr (std::is_same<ValueType, std::monostate>::value) {
                return {};
            }
            else if constexpr (std::is_same<ValueType, std::string>::value) {
                return value;
            }
            else if constexpr (std::is_same<ValueType, bool>::value) {
                return value ? "true" : "false";
            }
            else {
                std::ostringstream stream;
                if constexpr (std::is_same<ValueType, double>::value) {
                    // Preserve enough digits to round-trip a double value.
                    stream << std::setprecision(
                        std::numeric_limits<double>::max_digits10
                    );
                }
                stream << value;
                return stream.str();
            }
        },
        value_
    );
}

} // namespace datatable