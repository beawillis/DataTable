#include <iostream>

#include "datatable/table.hpp"

int main()
{
    datatable::DataTable table;

    table.addColumn("Name");
    table.addColumn("Age");

    std::cout << "DataTable Example\n";
    std::cout << "-----------------\n";

    std::cout << "Rows: "
              << table.rowCount()
              << '\n';

    std::cout << "Columns: "
              << table.columnCount()
              << '\n';

    std::cout << "Column names:\n";

    for (const auto& name : table.columnNames()) {
        std::cout << "  - " << name << '\n';
    }

    return 0;
}