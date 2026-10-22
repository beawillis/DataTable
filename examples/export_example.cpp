#include <iostream>
#include <sstream>

#include "datatable/export.hpp"
#include "datatable/table.hpp"

int main()
{
    const datatable::DataTable table = datatable::DataTable::fromCsv(
        "Name,Note\n"
        "Ada,\"Uses, commas\"\n"
        "Grace,\"Research <data>\"\n"
    );

    std::ostringstream csv;
    std::ostringstream html;
    // Export writes complete representations without changing the table.
    datatable::write_csv(table, csv);
    datatable::write_html(table, html);

    std::cout << "Export Example\n";
    std::cout << "--------------\n";
    std::cout << "CSV output:\n" << csv.str();
    std::cout << "\nHTML output:\n" << html.str();

    return 0;
}