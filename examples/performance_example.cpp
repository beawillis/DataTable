#include <chrono>
#include <cstdint>
#include <iostream>

#include "datatable/aggregate.hpp"
#include "datatable/filter.hpp"
#include "datatable/group.hpp"
#include "datatable/sort.hpp"
#include "datatable/table.hpp"

int main()
{
    // Use a fixed input size so repeated smoke-test runs are comparable.
    constexpr std::size_t row_count = 10000;
    datatable::DataTable table;
    table.addColumn("Id");
    table.addColumn("Department");
    table.addColumn("Score");

    for (std::size_t index = 0; index < row_count; ++index) {
        // Build realistic mixed-type rows with two repeatable departments.
        table.addRow(datatable::Row{
            static_cast<std::uint64_t>(index),
            index % 2 == 0 ? "Engineering" : "Support",
            static_cast<double>(index % 1000) / 10.0
        });
    }

    // Measure only the integrated filtering, sorting, grouping, and
    // aggregation pipeline, not table construction.
    const auto started = std::chrono::steady_clock::now();
    datatable::DataTable selected =
        datatable::filterEquals(table, 1, datatable::Cell("Engineering"));
    datatable::sortByColumn(selected, 2);
    const auto groups = datatable::groupBy(selected, 1);
    const double average = datatable::mean(selected, 2);
    const auto elapsed = std::chrono::steady_clock::now() - started;
    const auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    std::cout << "Rows processed: " << row_count << '\n'
              << "Rows selected: " << selected.rowCount() << '\n'
              << "Groups: " << groups.size() << '\n'
              << "Average score: " << average << '\n'
              << "Pipeline time (ms): " << milliseconds << '\n';
    return 0;
}
