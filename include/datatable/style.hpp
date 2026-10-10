#ifndef DATATABLE_STYLE_HPP
#define DATATABLE_STYLE_HPP

#include <cstddef>

namespace datatable {

enum class Alignment
{
    left,
    center,
    right
};

// Stores presentation choices without owning or changing table data.
class Style
{
public:
    Style() = default;

    Alignment alignment() const noexcept;
    void setAlignment(Alignment alignment) noexcept;

    bool showHeader() const noexcept;
    void setShowHeader(bool show) noexcept;

    bool showBorders() const noexcept;
    void setShowBorders(bool show) noexcept;

    std::size_t padding() const noexcept;
    void setPadding(std::size_t padding) noexcept;

private:
    Alignment alignment_{Alignment::left};
    bool show_header_{true};
    bool show_borders_{true};
    std::size_t padding_{1};
};

} // namespace datatable

#endif // DATATABLE_STYLE_HPP