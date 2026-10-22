#include "datatable/column.hpp"

#include "test_assert.hpp"
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
    assert(names.type() == datatable::CellType::string);
    assert(names.at(0).get<std::string>() == "Ada");
    names.set(1, Cell("Hopper"));
    assert(names.at(1).toString() == "Hopper");

    names.append("Margaret");
    assert(names.size() == 3);
    assert(names.at(2).get<std::string>() == "Margaret");
    assert(names.cells().size() == 3);

    bool mixed_type_rejected = false;
    try {
        names.append(42);
    }
    catch (const std::invalid_argument&) {
        mixed_type_rejected = true;
    }
    assert(mixed_type_rejected);

    bool mismatched_update_rejected = false;
    try {
        names.set(0, 42);
    }
    catch (const std::invalid_argument&) {
        mismatched_update_rejected = true;
    }
    assert(mismatched_update_rejected);
    assert(names[0].get<std::string>() == "Ada");

    // A declared type remains available for an empty column.
    Column typed_empty("Count", datatable::CellType::signed_integer);
    assert(typed_empty.type() == datatable::CellType::signed_integer);
    typed_empty.append(nullptr);
    typed_empty.append(42);
    assert(typed_empty.size() == 2);

    bool null_type_rejected = false;
    try {
        const Column invalid_type("Invalid", datatable::CellType::null);
        (void)invalid_type;
    }
    catch (const std::invalid_argument&) {
        null_type_rejected = true;
    }
    assert(null_type_rejected);

    bool mixed_constructor_rejected = false;
    try {
        const Column invalid_values("Mixed", {Cell{"text"}, Cell{42}});
        (void)invalid_values;
    }
    catch (const std::invalid_argument&) {
        mixed_constructor_rejected = true;
    }
    assert(mixed_constructor_rejected);

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
    assert(names.type() == datatable::CellType::string);

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