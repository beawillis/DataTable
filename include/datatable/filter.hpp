#ifndef DATATABLE_FILTER_HPP
#define DATATABLE_FILTER_HPP

#include <functional>

#include "datatable/column.hpp"

namespace datatable {

// Return a copy containing only cells accepted by the predicate.
Column filterColumn(const Column& column,
                    const std::function<bool(const Cell&)>& predicate);

// Convenience filter for selecting cells equal to a supplied value.
Column filterEquals(const Column& column, const Cell& expected);

} // namespace datatable

#endif // DATATABLE_FILTER_HPP