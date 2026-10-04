#ifndef DATATABLE_TABLE_HPP
#define DATATABLE_TABLE_HPP

#include <cstddef>
#include <string>
#include <vector>

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

    /**
     * @brief Add a column name to the table schema.
     *
     * This is part of the initial architecture and will later
     * be connected to the typed Column implementation.
     */
    void addColumn(const std::string& name);

private:

    std::vector<std::string> column_names_;
    std::size_t row_count_;
};

} // namespace datatable

#endif // DATATABLE_TABLE_HPP