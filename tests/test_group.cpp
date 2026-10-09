#include <cassert>
#include <iostream>

#include "datatable/group.hpp"

using datatable::Cell;
using datatable::Column;
using datatable::groupBy;

int main()
{
    const Column source{
        "Department",
        {Cell{"Support"}, Cell{"Engineering"}, Cell{"Support"},
         Cell{"Sales"}, Cell{"Engineering"}}
    };

    const auto groups = groupBy(source);

    // Groups appear in the order their keys first occur.
    assert(groups.size() == 3);
    assert(groups[0].key().get<std::string>() == "Support");
    assert(groups[1].key().get<std::string>() == "Engineering");
    assert(groups[2].key().get<std::string>() == "Sales");

    // Each group reports how many source cells share its key.
    assert(groups[0].count() == 2);
    assert(groups[1].count() == 2);
    assert(groups[2].count() == 1);

    // Grouping does not modify the source column.
    assert(source.size() == 5);

    std::cout << "Group tests passed.\n";
}