#include <cassert>
#include <iostream>
#include <stdexcept>

#include "datatable/filter.hpp"
#include "datatable/table.hpp"

using datatable::Cell;
using datatable::Column;
using datatable::DataTable;
using datatable::filterColumn;
using datatable::filterEquals;
using datatable::Row;

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

    const DataTable table = DataTable::fromCsv(
        "Name,Department\n"
        "Ada,Engineering\n"
        "Grace,Support\n"
        "Lin,Engineering\n"
    );
    const DataTable engineering = filterEquals(table, 1, Cell{"Engineering"});

    // Filtering complete rows preserves values from every column.
    assert(engineering.rowCount() == 2);
    assert(engineering.column("Name")[0].get<std::string>() == "Ada");
    assert(engineering.column("Name")[1].get<std::string>() == "Lin");
    assert(table.rowCount() == 3);

    bool invalid_index_threw = false;
    try {
        (void)filterEquals(table, 99, Cell{"Engineering"});
    }
    catch (const std::out_of_range&) {
        invalid_index_threw = true;
    }
    assert(invalid_index_threw);

    std::cout << "Filter tests passed.\n";
}