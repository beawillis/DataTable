#include <chrono>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "datatable/aggregate.hpp"
#include "datatable/export.hpp"
#include "datatable/filter.hpp"
#include "datatable/group.hpp"
#include "datatable/sort.hpp"
#include "datatable/table.hpp"

namespace {

std::string makeCsv(std::size_t rowCount)
{
    std::string csv{"Id,Department,Score\n"};
    csv.reserve(rowCount * 40);
    for (std::size_t index = 0; index < rowCount; ++index) {
        csv += std::to_string(index);
        csv += index % 2 == 0 ? ",Engineering," : ",Support,";
        // Keep one consistent floating-point type throughout the score column.
        csv += std::to_string(static_cast<double>(index % 10000) / 10.0);
        csv.push_back('\n');
    }
    return csv;
}

template <typename Operation>
long long measureMicroseconds(Operation&& operation)
{
    const auto started = std::chrono::steady_clock::now();
    operation();
    const auto elapsed = std::chrono::steady_clock::now() - started;
    return std::chrono::duration_cast<std::chrono::microseconds>(
        elapsed
    ).count();
}

void runSample(std::size_t rowCount)
{
    // Generate input outside the loading timer so each measurement is scoped.
    const std::string csv = makeCsv(rowCount);
    datatable::DataTable table;
    const long long loadTime = measureMicroseconds([&] {
        table = datatable::DataTable::fromCsv(csv);
    });

    datatable::DataTable selected;
    const long long filterTime = measureMicroseconds([&] {
        selected = datatable::filterEquals(
            table,
            1,
            datatable::Cell{"Engineering"}
        );
    });

    // Keep the table copy outside the sort timer; sortByColumn includes its
    // internal row-ordering and synchronized-column work.
    datatable::DataTable sorted = selected;
    const long long sortTime = measureMicroseconds([&] {
        datatable::sortByColumn(sorted, 2);
    });

    datatable::GroupedColumn groups;
    const long long groupTime = measureMicroseconds([&] {
        groups = datatable::groupBy(table, 1);
    });

    double averageScore = 0.0;
    const long long aggregateTime = measureMicroseconds([&] {
        averageScore = datatable::mean(selected, 2);
    });

    std::ostringstream csvOutput;
    const long long csvExportTime = measureMicroseconds([&] {
        datatable::write_csv(selected, csvOutput);
    });

    std::ostringstream htmlOutput;
    const long long htmlExportTime = measureMicroseconds([&] {
        datatable::write_html(selected, htmlOutput);
    });

    if (table.rowCount() != rowCount ||
        selected.rowCount() != rowCount / 2 ||
        groups.size() != 2 ||
        datatable::count(selected, 2) != selected.rowCount() ||
        csvOutput.tellp() <= 0 ||
        htmlOutput.tellp() <= 0) {
        throw std::runtime_error("Performance example validation failed.");
    }

    std::cout << "Rows: " << rowCount
              << ", selected: " << selected.rowCount()
              << ", groups: " << groups.size()
              << ", average score: " << averageScore << '\n'
              << "  CSV load: " << loadTime << " us\n"
              << "  Filter: " << filterTime << " us\n"
              << "  Sort: " << sortTime << " us\n"
              << "  Group: " << groupTime << " us\n"
              << "  Mean: " << aggregateTime << " us\n"
              << "  CSV export: " << csvExportTime << " us\n"
              << "  HTML export: " << htmlExportTime << " us\n";
}

} // namespace

int main()
{
    // Representative sizes make growth trends visible without claiming a
    // cross-library benchmark or machine-independent performance result.
    for (const std::size_t rowCount : {1000, 10000, 100000}) {
        runSample(rowCount);
    }
    return 0;
}
