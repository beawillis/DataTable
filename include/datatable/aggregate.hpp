#ifndef DATATABLE_AGGREGATE_HPP
#define DATATABLE_AGGREGATE_HPP

#include <cstddef>

#include "datatable/cell.hpp"
#include "datatable/column.hpp"

namespace datatable {

// Count non-null cells in a column.
std::size_t count(const Column& column);

// Calculate numeric summaries while ignoring null cells.
double sum(const Column& column);
double mean(const Column& column);

// Return the smallest or largest non-null cell using the column's value order.
Cell min(const Column& column);
Cell max(const Column& column);

} // namespace datatable

#endif // DATATABLE_AGGREGATE_HPP