#include <cassert>
#include <iostream>
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

    DataTable empty;
    empty.addColumn("Name");
    // A schema-only table still produces a visible formatted result.
    assert(!formatTable(empty).empty());

    std::cout << "Formatter tests passed.\n";
}