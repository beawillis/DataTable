#include <cassert>
#include <functional>
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

    // Null cells can be selected explicitly without changing the source.
    const Column with_missing{"Values", {Cell(nullptr), Cell(4), Cell(nullptr)}};
    const Column missing = filterEquals(with_missing, Cell(nullptr));
    assert(missing.size() == 2);
    assert(missing[0].isNull());
    assert(missing[1].isNull());
    assert(with_missing.size() == 3);

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
    assert(engineering.columnCount() == table.columnCount());
    assert(engineering.column("Department")[0].get<std::string>() ==
           "Engineering");

    // An empty predicate is rejected explicitly instead of failing during
    // iteration with an unclear callable error.
    bool empty_predicate_threw = false;
    try {
        (void)filterColumn(source, std::function<bool(const Cell&)>{});
    }
    catch (const std::invalid_argument&) {
        empty_predicate_threw = true;
    }
    assert(empty_predicate_threw);

    bool empty_table_predicate_threw = false;
    try {
        (void)datatable::filterByColumn(
            table,
            0,
            std::function<bool(const Cell&)>{}
        );
    }
    catch (const std::invalid_argument&) {
        empty_table_predicate_threw = true;
    }
    assert(empty_table_predicate_threw);

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