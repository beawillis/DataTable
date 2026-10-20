#include "datatable/formatter.hpp"

#include "datatable/table.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

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

std::string formatCells(const std::vector<std::string>& values,
                        const Style& style)
{
    std::size_t width = 0;
    for (const std::string& value : values) {
        width = std::max(width, value.size());
    }

    std::ostringstream output;
    const auto writeRow = [&output, &style, width](const std::string& value) {
        output << "|"
               << std::string(style.padding(), ' ')
               << alignText(value, width, style.alignment())
               << std::string(style.padding(), ' ')
               << "|\n";
    };

    if (style.showBorders()) {
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

std::string displayValue(const Cell& cell, const Style& style)
{
    // Only doubles need formatter-controlled precision; other cell types
    // retain their normal textual representation.
    if (!style.hasDecimalPrecision() || !cell.holds<double>()) {
        return cell.toString();
    }

    std::ostringstream output;
    output << std::fixed << std::setprecision(style.decimalPrecision())
           << cell.get<double>();
    return output.str();
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
        values.push_back(displayValue(cell, style));
    }

    return formatCells(values, style);
}

std::string formatTable(const DataTable& table, const Style& style)
{
    if (table.columnCount() == 0) {
        return {};
    }
    // A footer is rendered as one complete table row, so its width must
    // match the table schema before any output is produced.
    if (!style.footer().empty() &&
        style.footer().size() != table.columnCount()) {
        throw std::invalid_argument(
            "Footer width must match the number of table columns."
        );
    }

    std::vector<std::size_t> widths(table.columnCount(), 0);
    for (std::size_t index = 0; index < table.columnCount(); ++index) {
        widths[index] = table.column(index).name().size();
        // Include formatted values so decimal precision affects alignment.
        for (const Row& row : table.rows()) {
            widths[index] = std::max(
                widths[index],
                displayValue(row[index], style).size()
            );
        }
        if (!style.footer().empty()) {
            // Footer labels can be wider than headers or data values.
            widths[index] = std::max(
                widths[index],
                style.footer()[index].size()
            );
        }
    }

    std::ostringstream output;
    const auto writeRow = [&output, &style, &widths](
                              const std::vector<std::string>& values) {
        output << "|";
        for (std::size_t index = 0; index < values.size(); ++index) {
            output << std::string(style.padding(), ' ')
                   << alignText(values[index], widths[index], style.alignment())
                   << std::string(style.padding(), ' ')
                   << "|";
        }
        output << "\n";
    };
    const auto writeBorder = [&output, &style, &widths]() {
        output << "+";
        for (const std::size_t width : widths) {
            output << std::string(width + style.padding() * 2, '-') << "+";
        }
        output << "\n";
    };

    if (style.showBorders()) {
        writeBorder();
    }
    if (style.showHeader()) {
        std::vector<std::string> headers;
        headers.reserve(table.columnCount());
        for (const Column& column : table.columns()) {
            headers.push_back(column.name());
        }
        writeRow(headers);
    }
    for (const Row& row : table.rows()) {
        std::vector<std::string> values;
        values.reserve(row.size());
        for (const Cell& cell : row.cells()) {
            values.push_back(displayValue(cell, style));
        }
        writeRow(values);
    }
    if (!style.footer().empty()) {
        // Render the footer separately; it is never stored as a data row.
        writeRow(style.footer());
    }
    if (style.showBorders()) {
        writeBorder();
    }

    return output.str();
}

} // namespace datatable