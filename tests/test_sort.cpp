#include <cassert>
#include <iostream>

#include "datatable/sort.hpp"

using datatable::Cell;
using datatable::Column;
using datatable::SortOrder;
using datatable::sortColumn;

int main()
{
    const Column source{"Scores", {Cell{3}, Cell{1}, Cell{2}}};

    // The default order is ascending and the source remains unchanged.
    const Column ascending = sortColumn(source);
    assert(ascending[0].get<std::int64_t>() == 1);
    assert(ascending[1].get<std::int64_t>() == 2);
    assert(ascending[2].get<std::int64_t>() == 3);
    assert(source[0].get<std::int64_t>() == 3);

    // An explicit descending order reverses the comparison direction.
    const Column descending = sortColumn(source, SortOrder::descending);
    assert(descending[0].get<std::int64_t>() == 3);
    assert(descending[2].get<std::int64_t>() == 1);

    std::cout << "Sort tests passed.\n";
}