#include <cassert>
#include <iostream>
#include <stdexcept>

#include "datatable/table.hpp"

using datatable::DataTable;
using datatable::Row;

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

void test_add_rows_and_access_columns()
{
    DataTable table;
    table.addColumn("Name");
    table.addColumn("Age");

    table.addRow(Row{{"Ada", 36}});
    table.addRow(Row{{"Grace", 28}});

    assert(table.rowCount() == 2);
    assert(table.rows().size() == 2);
    assert(table.column("Name")[1].get<std::string>() == "Grace");
    assert(table.column(1)[0].get<std::int64_t>() == 36);
}

void test_invalid_schema_operations()
{
    DataTable table;
    table.addColumn("Name");

    bool duplicate_rejected = false;
    try {
        table.addColumn("Name");
    }
    catch (const std::invalid_argument&) {
        duplicate_rejected = true;
    }
    assert(duplicate_rejected);

    bool width_rejected = false;
    try {
        table.addRow(Row{{"Ada", 36}});
    }
    catch (const std::invalid_argument&) {
        width_rejected = true;
    }
    assert(width_rejected);

    table.addRow(Row{{"Ada"}});
    bool late_column_rejected = false;
    try {
        table.addColumn("Age");
    }
    catch (const std::logic_error&) {
        late_column_rejected = true;
    }
    assert(late_column_rejected);

    bool missing_column_rejected = false;
    try {
        (void)table.column("Missing");
    }
    catch (const std::out_of_range&) {
        missing_column_rejected = true;
    }
    assert(missing_column_rejected);
}

void test_csv_integration()
{
    const DataTable table = DataTable::fromCsv(
        "Name,Age,Active,Score\n"
        "Ada,36,true,98.5\n"
        "Grace,28,false,91.25\n"
    );

    assert(table.columnCount() == 4);
    assert(table.rowCount() == 2);
    assert(table.column("Name")[0].get<std::string>() == "Ada");
    assert(table.column("Age")[1].get<std::int64_t>() == 28);
    assert(table.column("Active")[1].get<bool>() == false);
    assert(table.column("Score")[0].get<double>() == 98.5);
}

void test_csv_width_validation()
{
    bool exception_thrown = false;
    try {
        (void)DataTable::fromCsv("Name,Age\nAda\n");
    }
    catch (const std::invalid_argument&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

int main()
{
    test_empty_table();
    test_add_column();
    test_empty_column_name();
    test_clear();
    test_add_rows_and_access_columns();
    test_invalid_schema_operations();
    test_csv_integration();
    test_csv_width_validation();

    std::cout << "All DataTable core tests passed.\n";

    return 0;
}