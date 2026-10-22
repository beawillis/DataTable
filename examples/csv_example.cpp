#include <iostream>
#include <string>

#include "datatable/formatter.hpp"
#include "datatable/table.hpp"

int main()
{
    const std::string input =
        "Name,Department,Salary\n"
        "Ada,Engineering,125000.0\n"
        "Grace,Research,132500.5\n"
        "Linus,Engineering,119000.0\n";

    // fromCsv performs parsing, type inference, and table construction together.
    const datatable::DataTable table = datatable::DataTable::fromCsv(input);

    std::cout << "CSV Example\n";
    std::cout << "-----------\n";
    std::cout << datatable::formatTable(table);

    return 0;
}