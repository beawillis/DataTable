#include <iostream>

#include "datatable/formatter.hpp"
#include "datatable/table.hpp"

int main()
{
    datatable::DataTable table;

    table.addColumn("Name");
    table.addColumn("Age");
    table.addColumn("Active");

    table.addRow(datatable::Row{
        datatable::Cell{"Ada"},
        datatable::Cell{37},
        datatable::Cell{true}
    });
    table.addRow(datatable::Row{
        datatable::Cell{"Grace"},
        datatable::Cell{42},
        datatable::Cell{false}
    });

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

    // Formatting is read-only, so this is safe for displaying the same table.
    std::cout << "\nFormatted table:\n";
    std::cout << datatable::formatTable(table);

    return 0;
}