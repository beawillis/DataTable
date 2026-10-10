#include <cassert>
#include <iostream>

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

    std::cout << "Style tests passed.\n";
}