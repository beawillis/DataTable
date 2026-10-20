#include "datatable/style.hpp"

#include <utility>

namespace datatable {
// Implementation of the Style class methods.
Alignment Style::alignment() const noexcept
{
    return alignment_;
}

void Style::setAlignment(Alignment alignment) noexcept
{
    // Store the requested alignment for the next formatting operation.
    alignment_ = alignment;
}

bool Style::showHeader() const noexcept
{
    return show_header_;
}

void Style::setShowHeader(bool show) noexcept
{
    // Toggle whether formatters should include a column header.
    show_header_ = show;
}

bool Style::showBorders() const noexcept
{
    return show_borders_;
}

void Style::setShowBorders(bool show) noexcept
{
    // Toggle the horizontal and vertical table decorations.
    show_borders_ = show;
}

std::size_t Style::padding() const noexcept
{
    return padding_;
}

void Style::setPadding(std::size_t padding) noexcept
{
    // Set the number of spaces around each displayed value.
    padding_ = padding;
}

bool Style::hasDecimalPrecision() const noexcept
{
    // Separate the enabled state from the precision value so zero is valid.
    return has_decimal_precision_;
}

std::size_t Style::decimalPrecision() const noexcept
{
    return decimal_precision_;
}

void Style::setDecimalPrecision(std::size_t precision) noexcept
{
    // Enabling precision makes the formatter use fixed decimal output.
    has_decimal_precision_ = true;
    decimal_precision_ = precision;
}

void Style::clearDecimalPrecision() noexcept
{
    // Restore the normal Cell::toString() representation.
    has_decimal_precision_ = false;
}

const std::vector<std::string>& Style::footer() const noexcept
{
    return footer_;
}

void Style::setFooter(std::vector<std::string> footer)
{
    // Move display-only labels into the style without changing table rows.
    footer_ = std::move(footer);
}

void Style::clearFooter() noexcept
{
    // Remove the optional footer while preserving all other style settings.
    footer_.clear();
}

} // namespace datatable