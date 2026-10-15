#ifndef DATATABLE_TABLE_HPP
#define DATATABLE_TABLE_HPP

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

#include "datatable/column.hpp"
#include "datatable/row.hpp"

namespace datatable {

/**
 * @brief Represents the central tabular data structure.
 *
 * DataTable provides the high-level interface through which
 * columns and rows are managed. Specialized functionality such
 * as CSV loading, sorting, filtering, grouping, formatting and
 * exporting is implemented by separate modules.
 */
class DataTable
{
public:

    /**
     * @brief Construct an empty DataTable.
     */
    DataTable();

    // Build a typed table from CSV records using the existing CSV parser.
    static DataTable fromCsv(std::istream& input);
    static DataTable fromCsv(const std::string& input);

    /**
     * @brief Return the number of rows in the table.
     */
    std::size_t rowCount() const noexcept;

    /**
     * @brief Return the number of columns in the table.
     */
    std::size_t columnCount() const noexcept;

    /**
     * @brief Check whether the table contains no rows.
     */
    bool empty() const noexcept;

    /**
     * @brief Remove all rows and columns from the table.
     */
    void clear() noexcept;

    /**
     * @brief Return the names of all columns.
     */
    const std::vector<std::string>& columnNames() const noexcept;

    // Add a named column; adding columns after rows exist is rejected.
    void addColumn(const std::string& name);

    // Add a row whose width matches the current number of columns.
    void addRow(const Row& row);

    // Replace the row order while keeping the column view synchronized.
    void replaceRows(const std::vector<Row>& rows);

    const std::vector<Row>& rows() const noexcept;
    const std::vector<Column>& columns() const noexcept;

    Column& column(std::size_t index);
    const Column& column(std::size_t index) const;

    Column& column(const std::string& name);
    const Column& column(const std::string& name) const;

private:

    std::vector<std::string> column_names_;
    std::vector<Column> columns_;
    std::vector<Row> rows_;
    std::size_t row_count_;
};

} // namespace datatable

#endif // DATATABLE_TABLE_HPP