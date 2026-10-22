#include "datatable/column.hpp"

#include <cassert>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using datatable::Cell;
using datatable::Column;

int main()
{
    static_assert(std::is_nothrow_move_constructible<Column>::value);

    // A column keeps its name and supports empty and populated states.
    Column empty("Name");
    assert(empty.name() == "Name");
    assert(empty.size() == 0);
    assert(empty.empty());

    Column names("Name", {"Ada", "Grace"});
    assert(names.size() == 2);
    assert(!names.empty());
    assert(names.at(0).get<std::string>() == "Ada");
    names[1] = Cell("Hopper");
    assert(names.at(1).toString() == "Hopper");

    names.append(42);
    assert(names.size() == 3);
    assert(names.at(2).get<std::int64_t>() == 42);
    assert(names.cells().size() == 3);

    // Reject invalid names and out-of-range cell access.
    bool empty_name_threw = false;
    try {
        const Column invalid("");
        (void)invalid;
    }
    catch (const std::invalid_argument&) {
        empty_name_threw = true;
    }
    assert(empty_name_threw);

    bool bounds_threw = false;
    try {
        (void)names.at(3);
    }
    catch (const std::out_of_range&) {
        bounds_threw = true;
    }
    assert(bounds_threw);

    // Clearing removes cells without invalidating the column.
    names.clear();
    assert(names.empty());

    // Moving transfers owned storage without requiring cell copies.
    Column moved = std::move(names);
    assert(moved.name() == "Name");
    assert(moved.empty());

    // A large column keeps its name and all appended values.
    Column large("Values");
    for (std::size_t index = 0; index < 10000; ++index) {
        large.append(static_cast<std::int64_t>(index));
    }
    assert(large.size() == 10000);
    assert(large.at(9999).get<std::int64_t>() == 9999);

    // The empty column remains a valid object after its cells are cleared.
    large.clear();
    assert(large.empty());
    assert(large.name() == "Values");
}