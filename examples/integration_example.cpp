#include <iostream>
#include <sstream>

#include "datatable/aggregate.hpp"
#include "datatable/export.hpp"
#include "datatable/filter.hpp"
#include "datatable/formatter.hpp"
#include "datatable/sort.hpp"
#include "datatable/table.hpp"

int main()
{
    // Use an in-memory CSV stream to demonstrate the public loading API.
    std::istringstream input(
        "Name,Department,Score\n"
        "Ada,Engineering,98.5\n"
        "Grace,Support,91.0\n"
        "Lin,Engineering,96.0\n"
    );

    const datatable::DataTable source =
        datatable::DataTable::fromCsv(input);
    // Filtering creates a new table while preserving complete records.
    datatable::DataTable engineering =
        datatable::filterEquals(source, 1, datatable::Cell("Engineering"));

    // Sorting reorders complete rows, keeping related values together.
    datatable::sortByColumn(
        engineering,
        2,
        datatable::SortOrder::descending
    );

    // Formatting and aggregation consume the same integrated table.
    std::cout << datatable::formatTable(engineering);
    std::cout << "Average score: "
              << datatable::mean(engineering, 2)
              << '\n';
    return 0;
}
