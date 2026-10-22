#include "datatable/row.hpp"

#include "test_assert.hpp"
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using datatable::Cell;
using datatable::Row;

int main()
{
    static_assert(std::is_nothrow_move_constructible<Row>::value);

    // Rows can be empty or initialized directly with mixed cell types.
    Row empty;
    assert(empty.size() == 0);
    assert(empty.empty());

    Row row{"Ada", 36, true};
    assert(row.size() == 3);
    assert(!row.empty());
    assert(row.at(0).get<std::string>() == "Ada");
    assert(row[1].get<std::int64_t>() == 36);
    assert(row.at(2).get<bool>());

    row[1] = Cell(37);
    assert(row.at(1).get<std::int64_t>() == 37);

    row.append(nullptr);
    assert(row.size() == 4);
    assert(row.at(3).isNull());
    assert(row.cells().size() == 4);

    // Invalid indices throw, and clearing leaves an empty row.
    bool bounds_threw = false;
    try {
        (void)row.at(4);
    }
    catch (const std::out_of_range&) {
        bounds_threw = true;
    }
    assert(bounds_threw);

    row.clear();
    assert(row.empty());

    // Row owns its vector and supports efficient move transfer.
    Row moved = std::move(row);
    assert(moved.empty());

    // A large row remains valid while cells are appended and indexed.
    Row large;
    for (std::size_t index = 0; index < 10000; ++index) {
        large.append(static_cast<std::int64_t>(index));
    }
    assert(large.size() == 10000);
    assert(large.at(9999).get<std::int64_t>() == 9999);

    // A wrong requested type is rejected without changing the row.
    bool wrong_type_threw = false;
    try {
        (void)large.at(0).get<std::string>();
    }
    catch (const std::bad_variant_access&) {
        wrong_type_threw = true;
    }
    assert(wrong_type_threw);
    assert(large.size() == 10000);
}