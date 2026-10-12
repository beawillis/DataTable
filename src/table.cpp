#include "datatable/table.hpp"

#include <algorithm>
#include <stdexcept>

namespace datatable {

DataTable::DataTable()
    : column_names_{},
      columns_{},
      rows_{},
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
    columns_.clear();
    rows_.clear();
    row_count_ = 0;
}

const std::vector<std::string>&
DataTable::columnNames() const noexcept
{
    return column_names_;
}

void DataTable::addColumn(const std::string& name)
{
    // Validate the name before changing either table representation.
    if (name.empty()) {
        throw std::invalid_argument(
            "Column name cannot be empty."
        );
    }

    if (!rows_.empty()) {
        // Existing rows cannot gain a new cell retroactively.
        throw std::logic_error(
            "Cannot add a column after rows have been added."
        );
    }

    if (std::find(column_names_.begin(), column_names_.end(), name) !=
        column_names_.end()) {
        // A name must identify exactly one column.
        throw std::invalid_argument("Duplicate column name.");
    }

    column_names_.push_back(name);
    columns_.emplace_back(name);
}

void DataTable::addRow(const Row& row)
{
    // Every row must align with the current schema.
    if (row.size() != columns_.size()) {
        throw std::invalid_argument(
            "Row width must match the number of table columns."
        );
    }

    rows_.push_back(row);
    // Keep row-oriented and column-oriented views synchronized.
    for (std::size_t index = 0; index < columns_.size(); ++index) {
        columns_[index].append(row[index]);
    }
    row_count_ = rows_.size();
}

const std::vector<Row>& DataTable::rows() const noexcept
{
    return rows_;
}

const std::vector<Column>& DataTable::columns() const noexcept
{
    return columns_;
}

Column& DataTable::column(std::size_t index)
{
    // vector::at reports an invalid column index explicitly.
    return columns_.at(index);
}

const Column& DataTable::column(std::size_t index) const
{
    return columns_.at(index);
}

Column& DataTable::column(const std::string& name)
{
    // Search the owned columns so callers can use a meaningful column name.
    for (Column& current : columns_) {
        if (current.name() == name) {
            return current;
        }
    }

    throw std::out_of_range("Column name was not found.");
}

const Column& DataTable::column(const std::string& name) const
{
    for (const Column& current : columns_) {
        if (current.name() == name) {
            return current;
        }
    }

    throw std::out_of_range("Column name was not found.");
}

} // namespace datatable