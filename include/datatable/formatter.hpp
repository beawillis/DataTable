#ifndef DATATABLE_FORMATTER_HPP
#define DATATABLE_FORMATTER_HPP

#include <string>

#include "datatable/column.hpp"
#include "datatable/style.hpp"

namespace datatable {

// Render one column as a readable table without changing its values.
std::string formatColumn(const Column& column,
                         const Style& style = Style{});

} // namespace datatable

#endif // DATATABLE_FORMATTER_HPP