#ifndef DATATABLE_FILTER_HPP
#define DATATABLE_FILTER_HPP

#include <functional>

#include "datatable/column.hpp"

namespace datatable {

class DataTable;

// Return a copy containing only cells accepted by the predicate.
Column filterColumn(const Column& column,
                    const std::function<bool(const Cell&)>& predicate);

// Convenience filter for selecting cells equal to a supplied value.
Column filterEquals(const Column& column, const Cell& expected);

// Return a table containing rows whose selected cell passes the predicate.
DataTable filterByColumn(
    const DataTable& table,
    std::size_t column_index,
    const std::function<bool(const Cell&)>& predicate
);

// Convenience table filter for selecting rows equal to a supplied value.
DataTable filterEquals(
    const DataTable& table,
    std::size_t column_index,
    const Cell& expected
);

} // namespace datatable

#endif // DATATABLE_FILTER_HPP