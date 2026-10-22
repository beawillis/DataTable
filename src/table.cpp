#include "datatable/table.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

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

    std::string storedName{name};
    Column column{storedName};
    column_names_.reserve(column_names_.size() + 1);
    columns_.reserve(columns_.size() + 1);
    column_names_.push_back(std::move(storedName));
    columns_.push_back(std::move(column));
}

void DataTable::addColumn(const std::string& name, CellType type)
{
    if (type == CellType::null) {
        throw std::invalid_argument(
            "A column cannot declare null as its value type."
        );
    }
    if (name.empty()) {
        throw std::invalid_argument("Column name cannot be empty.");
    }
    if (!rows_.empty()) {
        throw std::logic_error(
            "Cannot add a column after rows have been added."
        );
    }
    if (std::find(column_names_.begin(), column_names_.end(), name) !=
        column_names_.end()) {
        throw std::invalid_argument("Duplicate column name.");
    }

    std::string storedName{name};
    Column column{storedName, type};
    column_names_.reserve(column_names_.size() + 1);
    columns_.reserve(columns_.size() + 1);
    column_names_.push_back(std::move(storedName));
    columns_.push_back(std::move(column));
}

void DataTable::addRow(const Row& row)
{
    // Every row must align with the current schema.
    if (row.size() != columns_.size()) {
        throw std::invalid_argument(
            "Row width must match the number of table columns."
        );
    }

    for (std::size_t index = 0; index < columns_.size(); ++index) {
        columns_[index].validateCellType(row[index]);
    }

    // Stage copies and reserve all storage before changing either table view.
    Row stagedRow{row};
    std::vector<Cell> stagedCells{row.cells()};
    if (rows_.capacity() == rows_.size()) {
        const std::size_t doubledCapacity = rows_.capacity() == 0
            ? 1
            : (rows_.capacity() <= rows_.max_size() / 2
                ? rows_.capacity() * 2
                : rows_.max_size());
        rows_.reserve(std::max(rows_.size() + 1, doubledCapacity));
    }
    for (std::size_t index = 0; index < columns_.size(); ++index) {
        columns_[index].reserve(columns_[index].size() + 1);
    }

    rows_.push_back(std::move(stagedRow));
    for (std::size_t index = 0; index < columns_.size(); ++index) {
        columns_[index].append(std::move(stagedCells[index]));
    }
    row_count_ = rows_.size();
}

void DataTable::updateRow(std::size_t index, const Row& row)
{
    if (row.size() != columns_.size()) {
        throw std::invalid_argument(
            "Row width must match the number of table columns."
        );
    }
    (void)rows_.at(index);
    for (std::size_t columnIndex = 0;
         columnIndex < columns_.size();
         ++columnIndex) {
        columns_[columnIndex].validateCellType(row[columnIndex]);
    }

    // Copy first so allocation failures leave the table untouched.
    Row stagedRow{row};
    std::vector<Cell> stagedCells{row.cells()};
    rows_[index] = std::move(stagedRow);
    for (std::size_t columnIndex = 0;
         columnIndex < columns_.size();
         ++columnIndex) {
        columns_[columnIndex].set(
            index,
            std::move(stagedCells[columnIndex])
        );
    }
}

void DataTable::removeRow(std::size_t index)
{
    (void)rows_.at(index);
    rows_.erase(rows_.begin() + static_cast<std::ptrdiff_t>(index));
    for (Column& column : columns_) {
        column.cells_.erase(
            column.cells_.begin() + static_cast<std::ptrdiff_t>(index)
        );
    }
    row_count_ = rows_.size();
}

void DataTable::replaceRows(const std::vector<Row>& rows)
{
    for (const Row& row : rows) {
        if (row.size() != columns_.size()) {
            throw std::invalid_argument(
                "Row width must match the number of table columns."
            );
        }
        for (std::size_t index = 0; index < columns_.size(); ++index) {
            columns_[index].validateCellType(row[index]);
        }
    }

    std::vector<Row> stagedRows{rows};
    std::vector<Column> stagedColumns{columns_};
    for (Column& column : stagedColumns) {
        column.clear();
    }
    for (const Row& row : stagedRows) {
        for (std::size_t index = 0; index < stagedColumns.size(); ++index) {
            stagedColumns[index].append(row[index]);
        }
    }

    rows_.swap(stagedRows);
    columns_.swap(stagedColumns);
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

const Column& DataTable::column(std::size_t index) const
{
    return columns_.at(index);
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