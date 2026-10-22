#include <iostream>

#include "datatable/group.hpp"
#include "datatable/table.hpp"

int main()
{
    const datatable::DataTable table = datatable::DataTable::fromCsv(
        "Name,Department\n"
        "Ada,Engineering\n"
        "Grace,Research\n"
        "Linus,Engineering\n"
        "Margaret,Research\n"
    );

    // groupBy preserves first-seen key order and counts duplicate keys.
    const datatable::GroupedColumn groups = datatable::groupBy(table, 1);

    std::cout << "Grouping Example\n";
    std::cout << "----------------\n";
    for (const datatable::Group& group : groups) {
        std::cout << group.key().toString()
                  << ": "
                  << group.count()
                  << '\n';
    }

    return 0;
}