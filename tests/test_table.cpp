#include <cassert>
#include <iostream>
#include <stdexcept>

#include "datatable/table.hpp"

using datatable::DataTable;

void test_empty_table()
{
    DataTable table;

    assert(table.empty());
    assert(table.rowCount() == 0);
    assert(table.columnCount() == 0);
}

void test_add_column()
{
    DataTable table;

    table.addColumn("Name");
    table.addColumn("Age");

    assert(table.columnCount() == 2);
    assert(table.columnNames()[0] == "Name");
    assert(table.columnNames()[1] == "Age");
}

void test_empty_column_name()
{
    DataTable table;

    bool exception_thrown = false;

    try {
        table.addColumn("");
    }
    catch (const std::invalid_argument&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

void test_clear()
{
    DataTable table;

    table.addColumn("Name");
    table.addColumn("Age");

    table.clear();

    assert(table.empty());
    assert(table.columnCount() == 0);
    assert(table.rowCount() == 0);
}

int main()
{
    test_empty_table();
    test_add_column();
    test_empty_column_name();
    test_clear();

    std::cout << "All DataTable core tests passed.\n";

    return 0;
}