#include <iostream>
#include <sstream>

#include "datatable/export.hpp"
#include "datatable/table.hpp"

int main()
{
    const datatable::DataTable table = datatable::DataTable::fromCsv(
        "Name,Note\n"
        "Ada,\"Uses, commas\"\n"
        "Grace,Research data\n"
    );

    std::ostringstream csv;
    // CSV export writes the complete table without changing it.
    datatable::write_csv(table, csv);

    std::cout << "Export Example\n";
    std::cout << "--------------\n";
    std::cout << "CSV output:\n" << csv.str();

    return 0;
}