#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include "datatable/aggregate.hpp"
#include "datatable/table.hpp"

using datatable::Cell;
using datatable::Column;
using datatable::DataTable;
using datatable::Row;
using datatable::count;
using datatable::max;
using datatable::mean;
using datatable::min;
using datatable::sum;

int main()
{
    const Column scores{
        "Scores",
        {Cell{4}, Cell{nullptr}, Cell{2.5}, Cell{8}}
    };

    // Count reports values while excluding null cells.
    assert(count(scores) == 3);

    // Sum and mean use numeric values and ignore null cells.
    assert(std::abs(sum(scores) - 14.5) < 0.000001);
    assert(std::abs(mean(scores) - (14.5 / 3.0)) < 0.000001);

    // Minimum and maximum return the original numeric values.
    assert(min(scores).get<std::int64_t>() == 2);
    assert(max(scores).get<std::int64_t>() == 8);

    bool invalidInputRejected = false;
    try {
        (void)sum(Column{"Names", {Cell{"Ada"}}});
    }
    catch (const std::invalid_argument&) {
        invalidInputRejected = true;
    }
    assert(invalidInputRejected);

    const DataTable table = DataTable::fromCsv(
        "Name,Score\n"
        "Ada,4\n"
        "Grace,\n"
        "Lin,2.5\n"
        "Noah,8\n"
    );

    // Table overloads select a column and reuse the same null-aware logic.
    assert(count(table, 1) == 3);
    assert(std::abs(sum(table, 1) - 14.5) < 0.000001);
    assert(std::abs(mean(table, 1) - (14.5 / 3.0)) < 0.000001);
    assert(min(table, 1).get<std::int64_t>() == 2);
    assert(max(table, 1).get<std::int64_t>() == 8);

    bool invalidColumnRejected = false;
    try {
        (void)sum(table, 99);
    }
    catch (const std::out_of_range&) {
        invalidColumnRejected = true;
    }
    assert(invalidColumnRejected);

    DataTable empty;
    empty.addColumn("Score");
    bool emptyMeanRejected = false;
    try {
        (void)mean(empty, 0);
    }
    catch (const std::invalid_argument&) {
        emptyMeanRejected = true;
    }
    assert(emptyMeanRejected);

    std::cout << "Aggregate tests passed.\n";
}