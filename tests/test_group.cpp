#include <cassert>
#include <iostream>
#include <stdexcept>

#include "datatable/group.hpp"
#include "datatable/table.hpp"

using datatable::Cell;
using datatable::Column;
using datatable::DataTable;
using datatable::groupBy;
using datatable::Row;

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

    DataTable table;
    table.addColumn("Department");
    table.addColumn("Employee");
    table.addRow(Row{{"Support", "Ada"}});
    table.addRow(Row{{"Engineering", "Grace"}});
    table.addRow(Row{{"Support", "Lin"}});
    table.addRow(Row{{"Sales", "Noah"}});
    table.addRow(Row{{"Engineering", "Mia"}});

    const auto table_groups = groupBy(table, 0);
    assert(table_groups.size() == 3);
    assert(table_groups[0].key().get<std::string>() == "Support");
    assert(table_groups[0].count() == 2);
    assert(table_groups[1].key().get<std::string>() == "Engineering");
    assert(table_groups[1].count() == 2);
    assert(table_groups[2].key().get<std::string>() == "Sales");
    assert(table_groups[2].count() == 1);
    assert(table.rows()[0][1].get<std::string>() == "Ada");
    assert(table.column("Employee")[2].get<std::string>() == "Lin");

    DataTable empty;
    empty.addColumn("Department");
    assert(groupBy(empty, 0).empty());

    bool invalid_index_threw = false;
    try {
        (void)groupBy(table, 99);
    }
    catch (const std::out_of_range&) {
        invalid_index_threw = true;
    }
    assert(invalid_index_threw);

    std::cout << "Group tests passed.\n";
}