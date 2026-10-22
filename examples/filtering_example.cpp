#include <iostream>

#include "datatable/filter.hpp"
#include "datatable/formatter.hpp"
#include "datatable/table.hpp"

int main()
{
    const datatable::DataTable table = datatable::DataTable::fromCsv(
        "Name,Department,Salary\n"
        "Ada,Engineering,125000\n"
        "Grace,Research,132500\n"
        "Linus,Engineering,119000\n"
    );

    // Filtering returns a new table and leaves the source table unchanged.
    const datatable::DataTable engineering =
        datatable::filterEquals(table, 1, datatable::Cell{"Engineering"});

    std::cout << "Filtering Example\n";
    std::cout << "-----------------\n";
    std::cout << datatable::formatTable(engineering);

    return 0;
}