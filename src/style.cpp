#include "datatable/style.hpp"

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

} // namespace datatable