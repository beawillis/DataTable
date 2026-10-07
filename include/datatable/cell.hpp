#ifndef DATATABLE_CELL_HPP
#define DATATABLE_CELL_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>

namespace datatable {

// The supported value types; monostate represents a null cell.
using CellValue = std::variant<
    std::monostate,
    bool,
    std::int64_t,
    std::uint64_t,
    double,
    std::string
>;

class Cell
{
public:
    // Create a null cell or initialize it from a supported value type.
    Cell() noexcept = default;
    Cell(std::nullptr_t) noexcept;
    Cell(bool value) noexcept;
    Cell(std::string value);
    Cell(const char* value);

    // Store integers without losing signedness and floating-point values as double.
    template <
        typename Numeric,
        typename = std::enable_if_t<
            std::is_arithmetic<Numeric>::value &&
            !std::is_same<typename std::decay<Numeric>::type, bool>::value
        >
    >
    Cell(Numeric value) noexcept
    {
        if constexpr (std::is_integral<Numeric>::value) {
            if constexpr (std::is_signed<Numeric>::value) {
                value_ = static_cast<std::int64_t>(value);
            }
            else {
                value_ = static_cast<std::uint64_t>(value);
            }
        }
        else {
            value_ = static_cast<double>(value);
        }
    }

    // Check for null and inspect the stored value or its type.
    bool isNull() const noexcept;

    const CellValue& value() const noexcept;
    CellValue& value() noexcept;

    // Test for a type, then retrieve it; get throws if the requested type differs.
    template <typename T>
    bool holds() const noexcept
    {
        return std::holds_alternative<T>(value_);
    }

    template <typename T>
    const T& get() const
    {
        return std::get<T>(value_);
    }

    template <typename T>
    T& get()
    {
        return std::get<T>(value_);
    }

    // Retrieve a pointer to the value, or nullptr when the type does not match.
    template <typename T>
    const T* getIf() const noexcept
    {
        return std::get_if<T>(&value_);
    }

    template <typename T>
    T* getIf() noexcept
    {
        return std::get_if<T>(&value_);
    }

    // Format the stored value as text; null becomes an empty string.
    std::string toString() const;

private:
    CellValue value_{}; // Own the value directly; default construction is null.
};

} // namespace datatable

#endif // DATATABLE_CELL_HPP
