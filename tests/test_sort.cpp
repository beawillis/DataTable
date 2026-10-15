#include <cassert>
#include <iostream>
#include <stdexcept>

#include "datatable/sort.hpp"
#include "datatable/table.hpp"

using datatable::Cell;
using datatable::Column;
using datatable::DataTable;
using datatable::Row;
using datatable::SortOrder;
using datatable::sortByColumn;
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

    DataTable table;
    table.addColumn("Name");
    table.addColumn("Score");
    table.addColumn("Group");
    table.addRow(Row{{"Ada", 10, "A"}});
    table.addRow(Row{{"Grace", 2, "B"}});
    table.addRow(Row{{"Lin", 10, "C"}});

    sortByColumn(table, 1);
    assert(table.rows()[0][0].get<std::string>() == "Grace");
    assert(table.rows()[0][2].get<std::string>() == "B");
    assert(table.rows()[1][0].get<std::string>() == "Ada");
    assert(table.rows()[2][0].get<std::string>() == "Lin");
    assert(table.column("Name")[0].get<std::string>() == "Grace");

    sortByColumn(table, 1, SortOrder::descending);
    assert(table.rows()[0][0].get<std::string>() == "Ada");
    assert(table.rows()[1][0].get<std::string>() == "Lin");
    assert(table.rows()[2][0].get<std::string>() == "Grace");

    DataTable strings;
    strings.addColumn("Name");
    strings.addRow(Row{{"Zed"}});
    strings.addRow(Row{{"Ada"}});
    strings.addRow(Row{{"Mia"}});
    sortByColumn(strings, 0);
    assert(strings.rows()[0][0].get<std::string>() == "Ada");
    assert(strings.rows()[2][0].get<std::string>() == "Zed");

    DataTable empty;
    empty.addColumn("Value");
    sortByColumn(empty, 0);
    assert(empty.empty());

    bool invalid_index_threw = false;
    try {
        sortByColumn(table, 99);
    }
    catch (const std::out_of_range&) {
        invalid_index_threw = true;
    }
    assert(invalid_index_threw);

    std::cout << "Sort tests passed.\n";
}