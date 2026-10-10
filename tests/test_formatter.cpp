#include <cassert>
#include <iostream>
#include <string>

#include "datatable/formatter.hpp"

using datatable::Alignment;
using datatable::Cell;
using datatable::Column;
using datatable::Style;
using datatable::formatColumn;

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

    std::cout << "Formatter tests passed.\n";
}