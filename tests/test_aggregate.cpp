#include "test_assert.hpp"
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
        {Cell{4.0}, Cell{nullptr}, Cell{2.5}, Cell{8.0}}
    };

    // Count reports values while excluding null cells.
    assert(count(scores) == 3);

    // Sum and mean use numeric values and ignore null cells.
    assert(std::abs(sum(scores) - 14.5) < 0.000001);
    assert(std::abs(mean(scores) - (14.5 / 3.0)) < 0.000001);

    // Minimum and maximum return the original numeric values.
    assert(min(scores).get<double>() == 2.5);
    assert(max(scores).get<double>() == 8.0);

    // Numeric summaries also accept homogeneous boolean and unsigned columns.
    const Column boolean_values{"Flags", {Cell{true}, Cell{false}}};
    assert(sum(boolean_values) == 1.0);
    const Column unsigned_values{
        "Counts",
        {Cell{static_cast<std::uint64_t>(2)},
         Cell{static_cast<std::uint64_t>(6)}}
    };
    assert(sum(unsigned_values) == 8.0);
    assert(mean(unsigned_values) == 4.0);

    // Non-numeric non-null cells are invalid for every numeric summary.
    for (const auto operation : {0, 1, 2, 3}) {
        bool invalidInputRejected = false;
        try {
            const Column names{"Names", {Cell{"Ada"}}};
            if (operation == 0) {
                (void)sum(names);
            }
            else if (operation == 1) {
                (void)mean(names);
            }
            else if (operation == 2) {
                (void)min(names);
            }
            else {
                (void)max(names);
            }
        }
        catch (const std::invalid_argument&) {
            invalidInputRejected = true;
        }
        assert(invalidInputRejected);
    }

    const DataTable table = DataTable::fromCsv(
        "Name,Score\n"
        "Ada,4.0\n"
        "Grace,\n"
        "Lin,2.5\n"
        "Noah,8.0\n"
    );

    // Table overloads select a column and reuse the same null-aware logic.
    assert(count(table, 1) == 3);
    assert(std::abs(sum(table, 1) - 14.5) < 0.000001);
    assert(std::abs(mean(table, 1) - (14.5 / 3.0)) < 0.000001);
    assert(min(table, 1).get<double>() == 2.5);
    assert(max(table, 1).get<double>() == 8.0);

    bool invalidColumnRejected = false;
    try {
        (void)sum(table, 99);
    }
    catch (const std::out_of_range&) {
        invalidColumnRejected = true;
    }
    assert(invalidColumnRejected);

    // Null-only input has no meaningful mean, minimum, or maximum.
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

    for (const auto operation : {0, 1}) {
        bool emptyRejected = false;
        try {
            if (operation == 0) {
                (void)min(empty, 0);
            }
            else {
                (void)max(empty, 0);
            }
        }
        catch (const std::invalid_argument&) {
            emptyRejected = true;
        }
        assert(emptyRejected);
    }

    // Every table overload must reject an invalid column index.
    for (const auto operation : {0, 1, 2, 3}) {
        bool invalidIndexRejected = false;
        try {
            if (operation == 0) {
                (void)count(table, 99);
            }
            else if (operation == 1) {
                (void)mean(table, 99);
            }
            else if (operation == 2) {
                (void)min(table, 99);
            }
            else {
                (void)max(table, 99);
            }
        }
        catch (const std::out_of_range&) {
            invalidIndexRejected = true;
        }
        assert(invalidIndexRejected);
    }

    std::cout << "Aggregate tests passed.\n";
}