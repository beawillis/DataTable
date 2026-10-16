#ifndef DATATABLE_AGGREGATE_HPP
#define DATATABLE_AGGREGATE_HPP

#include <cstddef>

#include "datatable/cell.hpp"
#include "datatable/column.hpp"

namespace datatable {

class DataTable;

// Count non-null cells in a column.
std::size_t count(const Column& column);
std::size_t count(const DataTable& table, std::size_t column_index);

// Calculate numeric summaries while ignoring null cells.
double sum(const Column& column);
double mean(const Column& column);
double sum(const DataTable& table, std::size_t column_index);
double mean(const DataTable& table, std::size_t column_index);

// Return the smallest or largest non-null cell using the column's value order.
Cell min(const Column& column);
Cell max(const Column& column);
Cell min(const DataTable& table, std::size_t column_index);
Cell max(const DataTable& table, std::size_t column_index);

} // namespace datatable

#endif // DATATABLE_AGGREGATE_HPP