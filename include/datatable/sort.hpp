#ifndef DATATABLE_SORT_HPP
#define DATATABLE_SORT_HPP

#include <cstddef>

#include "datatable/column.hpp"

namespace datatable {

class DataTable;

// Select the direction used when sorting values or table rows.
enum class SortOrder
{
    ascending,
    descending
};

// Return a stable, sorted copy; the source column is not modified.
Column sortColumn(const Column& column,
                  SortOrder order = SortOrder::ascending);

// Reorder table rows by the selected column while preserving row integrity.
void sortByColumn(DataTable& table,
                  std::size_t column_index,
                  SortOrder order = SortOrder::ascending);

} // namespace datatable

#endif // DATATABLE_SORT_HPP