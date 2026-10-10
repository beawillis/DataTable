#ifndef DATATABLE_SORT_HPP
#define DATATABLE_SORT_HPP

#include "datatable/column.hpp"

namespace datatable {

// Select the direction used when returning a sorted copy of a column.
enum class SortOrder
{
    ascending,
    descending
};

// Return a stable, sorted copy; the source column is not modified.
Column sortColumn(const Column& column,
                  SortOrder order = SortOrder::ascending);

} // namespace datatable

#endif // DATATABLE_SORT_HPP