#include "datatable/formatter.hpp"

#include <algorithm>
#include <sstream>

namespace datatable {
namespace {

std::string alignText(const std::string& text,
                      std::size_t width,
                      Alignment alignment)
{
    // Add spaces on the selected side while keeping every cell the same width.
    const std::size_t missing = width - text.size();
    if (alignment == Alignment::right) {
        return std::string(missing, ' ') + text;
    }
    if (alignment == Alignment::center) {
        const std::size_t left = missing / 2;
        return std::string(left, ' ') + text +
            std::string(missing - left, ' ');
    }
    return text + std::string(missing, ' ');
}

std::string border(std::size_t width)
{
    // Build a horizontal border matching the content width.
    return "+" + std::string(width + 2, '-') + "+\n";
}

} // namespace

std::string formatColumn(const Column& column, const Style& style)
{
    std::vector<std::string> values;
    values.reserve(column.size() + 1);

    // The header is optional; cell values are always rendered in source order.
    if (style.showHeader()) {
        values.push_back(column.name());
    }
    for (const Cell& cell : column.cells()) {
        values.push_back(cell.toString());
    }

    std::size_t width = 0;
    // Use the longest displayed value so no row is truncated.
    for (const std::string& value : values) {
        width = std::max(width, value.size());
    }

    std::ostringstream output;
    const auto writeRow = [&output, &style, width](const std::string& value) {
        // Render one aligned row with the configured padding.
        output << "|"
               << std::string(style.padding(), ' ')
               << alignText(value, width, style.alignment())
               << std::string(style.padding(), ' ')
               << "|\n";
    };

    if (style.showBorders()) {
        // Draw the top border before the first row.
        output << border(width + style.padding() * 2);
    }
    for (std::size_t index = 0; index < values.size(); ++index) {
        writeRow(values[index]);
        if (style.showBorders() &&
            (index + 1 == values.size() || !style.showHeader())) {
            output << border(width + style.padding() * 2);
        }
    }

    return output.str();
}

} // namespace datatable