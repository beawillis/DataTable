#ifndef DATATABLE_STYLE_HPP
#define DATATABLE_STYLE_HPP

#include <cstddef> // std::size_t
#include <string>
#include <vector>

namespace datatable {

enum class Alignment // Specifies the alignment of text within a cell
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

    // Precision is optional so default formatting preserves Cell text.
    bool hasDecimalPrecision() const noexcept;
    std::size_t decimalPrecision() const noexcept;
    // Apply fixed precision only when formatting floating-point cells.
    void setDecimalPrecision(std::size_t precision) noexcept;
    void clearDecimalPrecision() noexcept;

    // Footer values are display text and are not added to the DataTable.
    const std::vector<std::string>& footer() const noexcept;
    void setFooter(std::vector<std::string> footer);
    void clearFooter() noexcept;

private:
    Alignment alignment_{Alignment::left};
    bool show_header_{true};
    bool show_borders_{true};
    std::size_t padding_{1};
    bool has_decimal_precision_{false};
    std::size_t decimal_precision_{0};
    std::vector<std::string> footer_;
};

} // namespace datatable

#endif // DATATABLE_STYLE_HPP