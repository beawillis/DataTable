#include "test_assert.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "datatable/aggregate.hpp"
#include "datatable/export.hpp"
#include "datatable/filter.hpp"
#include "datatable/formatter.hpp"
#include "datatable/group.hpp"
#include "datatable/sort.hpp"
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

void test_row_update_and_removal()
{
    DataTable table;
    table.addColumn("Name", datatable::CellType::string);
    table.addColumn("Age", datatable::CellType::signed_integer);
    table.addRow(Row{{"Ada", 36}});
    table.addRow(Row{{"Grace", 28}});

    // Updating a row updates the row and every synchronized column view.
    table.updateRow(1, Row{{"Grace Hopper", 85}});
    assert(table.rows()[1][0].get<std::string>() == "Grace Hopper");
    assert(table.column("Name")[1].get<std::string>() == "Grace Hopper");
    assert(table.column("Age")[1].get<std::int64_t>() == 85);
    assert(table.rowCount() == 2);

    bool wrong_type_rejected = false;
    try {
        table.updateRow(0, Row{{"Ada", "thirty-six"}});
    }
    catch (const std::invalid_argument&) {
        wrong_type_rejected = true;
    }
    assert(wrong_type_rejected);
    assert(table.rows()[0][1].get<std::int64_t>() == 36);

    bool wrong_width_rejected = false;
    try {
        table.updateRow(0, Row{{"Ada"}});
    }
    catch (const std::invalid_argument&) {
        wrong_width_rejected = true;
    }
    assert(wrong_width_rejected);

    bool invalid_update_rejected = false;
    try {
        table.updateRow(99, Row{{"Nobody", 0}});
    }
    catch (const std::out_of_range&) {
        invalid_update_rejected = true;
    }
    assert(invalid_update_rejected);

    // Removal keeps the remaining row and both columns aligned.
    table.removeRow(0);
    assert(table.rowCount() == 1);
    assert(table.rows()[0][0].get<std::string>() == "Grace Hopper");
    assert(table.column("Name").size() == 1);
    assert(table.column("Age")[0].get<std::int64_t>() == 85);
    assert(table.column("Name").type() == datatable::CellType::string);

    bool invalid_remove_rejected = false;
    try {
        table.removeRow(1);
    }
    catch (const std::out_of_range&) {
        invalid_remove_rejected = true;
    }
    assert(invalid_remove_rejected);

    table.removeRow(0);
    assert(table.empty());
    assert(table.columnCount() == 2);
    assert(table.column("Age").type() == datatable::CellType::signed_integer);
}

void test_homogeneous_column_validation()
{
    DataTable table;
    table.addColumn("Score");
    table.addColumn("Note");
    table.addRow(Row{{1.5, "good"}});

    bool mixed_column_rejected = false;
    try {
        table.addRow(Row{{2, "also good"}});
    }
    catch (const std::invalid_argument&) {
        mixed_column_rejected = true;
    }
    assert(mixed_column_rejected);
    assert(table.rowCount() == 1);
    assert(table.column("Score").type() == datatable::CellType::floating_point);

    bool mixed_csv_rejected = false;
    try {
        (void)DataTable::fromCsv(
            "Score\n"
            "1.5\n"
            "2\n"
        );
    }
    catch (const std::invalid_argument&) {
        mixed_csv_rejected = true;
    }
    assert(mixed_csv_rejected);

    // Null values are accepted without weakening the established column type.
    table.addRow(Row{{nullptr, "missing score"}});
    assert(table.rowCount() == 2);
    assert(table.column("Score").type() == datatable::CellType::floating_point);
}

void test_full_module_integration()
{
    // Load one shared table so every module works with the same schema and
    // typed row data instead of creating incompatible table representations.
    DataTable table = DataTable::fromCsv(
        "Name,Department,Score\n"
        "Ada,Engineering,98.5\n"
        "Grace,Support,91.0\n"
        "Lin,Engineering,96.0\n"
    );

    const DataTable engineering =
        datatable::filterEquals(table, 1, datatable::Cell("Engineering"));
    // Filtering returns a new table containing complete matching rows.
    assert(engineering.rowCount() == 2);

    DataTable ordered = engineering;
    // Sort the working copy so the filtered result remains an independent
    // source for checking non-mutating behavior.
    datatable::sortByColumn(ordered, 2, datatable::SortOrder::descending);
    assert(ordered.column("Name")[0].get<std::string>() == "Ada");
    assert(ordered.column("Name")[1].get<std::string>() == "Lin");

    const auto groups = datatable::groupBy(ordered, 1);
    // Grouping and aggregation read the synchronized column view.
    assert(groups.size() == 1);
    assert(groups[0].count() == 2);
    assert(datatable::count(ordered, 2) == 2);
    assert(datatable::mean(ordered, 2) == 97.25);

    const std::string formatted = datatable::formatTable(ordered);
    // Formatting reads the table and returns presentation text only.
    assert(formatted.find("Engineering") != std::string::npos);

    std::ostringstream csv;
    // Exporters write the same complete rows in their respective formats.
    datatable::write_csv(ordered, csv);
    assert(csv.str().find("Ada,Engineering,98.5") != std::string::npos);

    std::ostringstream html;
    datatable::write_html(ordered, html);
    assert(html.str().find("<td>Ada</td>") != std::string::npos);

    // Sorting changes only the working copy, not the filtered source.
    assert(engineering.column("Name")[0].get<std::string>() == "Ada");
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
    test_row_update_and_removal();
    test_homogeneous_column_validation();
    test_full_module_integration();

    std::cout << "All DataTable core tests passed.\n";

    return 0;
}