#ifndef DATATABLE_FORMATTER_HPP
#define DATATABLE_FORMATTER_HPP

#include <string>

#include "datatable/column.hpp"
#include "datatable/style.hpp"

namespace datatable {

class DataTable;

// Render one column as a readable table without changing its values.
std::string formatColumn(const Column& column,
                         const Style& style = Style{});

// Render the complete table without changing its rows, columns, or cells.
std::string formatTable(const DataTable& table,
                        const Style& style = Style{});

} // namespace datatable

#endif // DATATABLE_FORMATTER_HPP