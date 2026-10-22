#include "datatable/cell.hpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

using datatable::Cell;

int main()
{
    static_assert(std::is_nothrow_move_constructible<Cell>::value);

    // A default cell represents a valid missing value rather than an error.
    // Default and explicit null values share the same representation.
    const Cell missing;
    assert(missing.isNull());
    assert(missing.toString().empty());

    const Cell null_value(nullptr);
    assert(null_value.isNull());

    // Preserve each supported primitive type and provide readable text.
    const Cell boolean(true);
    assert(boolean.holds<bool>());
    assert(boolean.get<bool>());
    assert(boolean.toString() == "true");

    const Cell signed_integer(-42);
    assert(signed_integer.holds<std::int64_t>());
    assert(signed_integer.get<std::int64_t>() == -42);
    assert(signed_integer.toString() == "-42");

    const Cell unsigned_integer(42U);
    assert(unsigned_integer.holds<std::uint64_t>());
    assert(unsigned_integer.get<std::uint64_t>() == 42U);

    const Cell decimal(3.5);
    assert(decimal.holds<double>());
    assert(decimal.toString() == "3.5");

    const Cell float_value(1.25F);
    assert(float_value.holds<double>());
    assert(float_value.get<double>() == 1.25);

    const Cell text("hello");
    assert(text.holds<std::string>());
    assert(text.get<std::string>() == "hello");
    assert(text.toString() == "hello");

    // Wrong typed access is reported, while getIf provides a non-throwing
    // type check for callers that do not know the stored type in advance.
    bool bad_access_threw = false;
    try {
        (void)text.get<double>();
    }
    catch (const std::bad_variant_access&) {
        bad_access_threw = true;
    }
    assert(bad_access_threw);
    assert(text.getIf<double>() == nullptr);
    assert(text.getIf<std::string>() != nullptr);

    bool null_string_threw = false;
    try {
        const Cell invalid(static_cast<const char*>(nullptr));
        (void)invalid;
    }
    catch (const std::invalid_argument&) {
        null_string_threw = true;
    }
    assert(null_string_threw);

    // Preserve the largest standard unsigned integer exactly.
    const Cell largest_integer(
        std::numeric_limits<unsigned long long>::max()
    );
    assert(largest_integer.holds<std::uint64_t>());

    // A long double beyond double's range must not silently become infinity.
    bool oversized_float_threw = false;
    try {
        const long double oversized =
            static_cast<long double>(std::numeric_limits<double>::max()) * 2;
        const Cell invalid(oversized);
        (void)invalid;
    }
    catch (const std::out_of_range&) {
        oversized_float_threw = true;
    }
    assert(oversized_float_threw);

    // Repeated construction verifies that valid cells remain independent and
    // stable when used in a larger collection.
    std::vector<Cell> many_cells;
    many_cells.reserve(10000);
    for (std::size_t index = 0; index < 10000; ++index) {
        many_cells.emplace_back(static_cast<std::int64_t>(index));
    }
    assert(many_cells.size() == 10000);
    assert(many_cells.back().get<std::int64_t>() == 9999);
}
