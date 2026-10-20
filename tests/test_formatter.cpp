#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

#include "datatable/formatter.hpp"
#include "datatable/table.hpp"

using datatable::Alignment;
using datatable::Cell;
using datatable::Column;
using datatable::DataTable;
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

    std::cout << "Formatter tests passed.\n";
}