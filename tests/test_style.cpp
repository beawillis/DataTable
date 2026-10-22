#include "test_assert.hpp"
#include <iostream>
#include <string>
#include <vector>

#include "datatable/style.hpp"

using datatable::Alignment;
using datatable::Style;

int main()
{
    Style style;
    assert(style.alignment() == Alignment::left);
    assert(style.showHeader());
    assert(style.showBorders());
    assert(style.padding() == 1);

    // Style changes affect presentation settings only.
    style.setAlignment(Alignment::right);
    style.setShowHeader(false);
    style.setShowBorders(false);
    style.setPadding(2);

    assert(style.alignment() == Alignment::right);
    assert(!style.showHeader());
    assert(!style.showBorders());
    assert(style.padding() == 2);

    style.setDecimalPrecision(3);
    assert(style.hasDecimalPrecision());
    assert(style.decimalPrecision() == 3);
    // Clearing precision restores the default display behavior.
    style.clearDecimalPrecision();
    assert(!style.hasDecimalPrecision());

    style.setFooter({"Total", "42"});
    const std::vector<std::string> expected_footer{"Total", "42"};
    assert(style.footer() == expected_footer);
    // Clearing a footer does not affect the other style settings.
    style.clearFooter();
    assert(style.footer().empty());

    std::cout << "Style tests passed.\n";
}