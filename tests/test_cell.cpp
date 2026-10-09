#include "datatable/cell.hpp"

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>

using datatable::Cell;

int main()
{
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

    // Incorrect typed access throws, and a null C-string is rejected.
    bool bad_access_threw = false;
    try {
        (void)text.get<double>();
    }
    catch (const std::bad_variant_access&) {
        bad_access_threw = true;
    }
    assert(bad_access_threw);

    bool null_string_threw = false;
    try {
        const Cell invalid(static_cast<const char*>(nullptr));
        (void)invalid;
    }
    catch (const std::invalid_argument&) {
        null_string_threw = true;
    }
    assert(null_string_threw);
}
