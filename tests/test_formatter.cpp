#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "datatable/formatter.hpp"
#include "datatable/table.hpp"

using datatable::Alignment;
using datatable::Cell;
using datatable::Column;
using datatable::DataTable;
using datatable::Row;
using datatable::Style;
using datatable::formatColumn;
using datatable::formatTable;

int main()
{
    const Column scores{"Scores", {Cell{1}, Cell{20}}};

    const std::string formatted = formatColumn(scores);
    // The default style includes a header, padding, and borders.
    assert(formatted.find("| Scores |") != std::string::npos);
    assert(formatted.find("| 1      |") != std::string::npos);

    Style compact;
    compact.setAlignment(Alignment::right);
    compact.setShowHeader(false);
    compact.setShowBorders(false);
    compact.setPadding(0);

    const std::string compactOutput = formatColumn(scores, compact);
    // A custom style can remove decoration and right-align the values.
    assert(compactOutput == "|1 \n|20\n");
    assert(scores.size() == 2);

    const DataTable table = DataTable::fromCsv(
        "Name,Score\n"
        "Ada,10\n"
        "Grace,200\n"
    );
    // A complete table keeps headers, rows, and columns aligned.
    const std::string tableOutput = formatTable(table);
    assert(tableOutput.find("| Name  | Score |") != std::string::npos);
    assert(tableOutput.find("| Ada   | 10    |") != std::string::npos);
    assert(tableOutput.find("| Grace | 200   |") != std::string::npos);

    Style noDecoration;
    noDecoration.setShowHeader(false);
    noDecoration.setShowBorders(false);
    noDecoration.setPadding(0);
    // Formatting options can produce a compact console representation.
    const std::string plainOutput = formatTable(table, noDecoration);
    assert(plainOutput == "|Ada  |10 \n|Grace|200\n");

    Style detailed;
    // Precision changes display text without changing stored cell values.
    detailed.setDecimalPrecision(2);
    // Footer content is presentation-only and follows the table schema.
    detailed.setFooter({"Total", "300.50"});
    const DataTable decimalTable = DataTable::fromCsv(
        "Name,Score\n"
        "Ada,10.5\n"
        "Grace,20\n"
    );
    const std::string detailedOutput = formatTable(decimalTable, detailed);
    assert(detailedOutput.find("10.50") != std::string::npos);
    assert(detailedOutput.find("20.00") != std::string::npos);
    assert(detailedOutput.find("Total") != std::string::npos);
    assert(detailedOutput.find("300.50") != std::string::npos);

    bool footer_width_threw = false;
    try {
        // A mismatched footer cannot be aligned with the table columns.
        Style invalidFooter;
        invalidFooter.setFooter({"Only one cell"});
        (void)formatTable(decimalTable, invalidFooter);
    }
    catch (const std::invalid_argument&) {
        footer_width_threw = true;
    }
    assert(footer_width_threw);

    DataTable empty;
    empty.addColumn("Name");
    // A schema-only table still produces a visible formatted result.
    assert(!formatTable(empty).empty());

    DataTable mixed;
    mixed.addColumn("ID");
    mixed.addColumn("Description with a deliberately long column name");
    mixed.addColumn("Value");
    mixed.addRow(Row{
        Cell{std::numeric_limits<std::int64_t>::max()},
        Cell{"short"},
        Cell{true}
    });
    mixed.addRow(Row{
        Cell{static_cast<std::uint64_t>(9007199254740991ULL)},
        Cell{"a much longer description value"},
        Cell{nullptr}
    });

    const auto beforeRows = mixed.rows();
    Style mixedStyle;
    mixedStyle.setDecimalPrecision(2);
    const std::string mixedOutput = formatTable(mixed, mixedStyle);
    // Width calculation must accommodate both short and long names/values.
    assert(mixedOutput.find("Description with a deliberately long column name")
           != std::string::npos);
    assert(mixedOutput.find("9223372036854775807") != std::string::npos);
    assert(mixedOutput.find("a much longer description value")
           != std::string::npos);
    // Formatting is presentation-only and must preserve rows and cell types.
    assert(mixed.rows().size() == beforeRows.size());
    for (std::size_t row = 0; row < beforeRows.size(); ++row) {
        assert(mixed.rows()[row].size() == beforeRows[row].size());
        for (std::size_t cell = 0; cell < beforeRows[row].size(); ++cell) {
            assert(mixed.rows()[row][cell].value()
                   == beforeRows[row][cell].value());
        }
    }
    const std::vector<std::string> expectedNames{
        "ID", "Description with a deliberately long column name", "Value"
    };
    assert(mixed.columnNames() == expectedNames);

    const DataTable noSchema;
    // A completely empty table has no columns to render.
    assert(formatTable(noSchema).empty());

    std::cout << "Formatter tests passed.\n";
}