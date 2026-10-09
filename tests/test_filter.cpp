#include <cassert>
#include <iostream>

#include "datatable/filter.hpp"

using datatable::Cell;
using datatable::Column;
using datatable::filterColumn;
using datatable::filterEquals;

int main()
{
    const Column source{"Names", {Cell{"Ada"}, Cell{"Grace"}, Cell{"Ada"}}};

    // Exact matching keeps both occurrences and preserves their order.
    const Column selected = filterEquals(source, Cell{"Ada"});
    assert(selected.size() == 2);
    assert(selected[0].get<std::string>() == "Ada");
    assert(selected[1].get<std::string>() == "Ada");
    assert(source.size() == 3);

    // A predicate can express a custom filtering rule.
    const Column longNames = filterColumn(
        source,
        [](const Cell& cell) {
            return cell.get<std::string>().size() > 3;
        });
    assert(longNames.size() == 1);
    assert(longNames[0].get<std::string>() == "Grace");

    std::cout << "Filter tests passed.\n";
}