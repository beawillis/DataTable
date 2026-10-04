#include "datatable/table.hpp"

#include <stdexcept>

namespace datatable {

DataTable::DataTable()
    : column_names_{},
      row_count_{0}
{
}

std::size_t DataTable::rowCount() const noexcept
{
    return row_count_;
}

std::size_t DataTable::columnCount() const noexcept
{
    return column_names_.size();
}

bool DataTable::empty() const noexcept
{
    return row_count_ == 0;
}

void DataTable::clear() noexcept
{
    column_names_.clear();
    row_count_ = 0;
}

const std::vector<std::string>&
DataTable::columnNames() const noexcept
{
    return column_names_;
}

void DataTable::addColumn(const std::string& name)
{
    if (name.empty()) {
        throw std::invalid_argument(
            "Column name cannot be empty."
        );
    }

    column_names_.push_back(name);
}

} // namespace datatable