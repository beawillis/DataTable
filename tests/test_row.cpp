#include "datatable/row.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

using datatable::Cell;
using datatable::Row;

int main()
{
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
}