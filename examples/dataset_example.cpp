#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "datatable/aggregate.hpp"
#include "datatable/csv.hpp"
#include "datatable/export.hpp"
#include "datatable/filter.hpp"
#include "datatable/formatter.hpp"
#include "datatable/group.hpp"
#include "datatable/sort.hpp"
#include "datatable/table.hpp"

namespace {

// strtoll handles signed values; try strtoull too so large unsigned integers
// are recognized without accidentally accepting negative inputs as unsigned.
bool isSupportedInteger(const std::string& value)
{
    if (value.empty()) {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    std::strtoll(value.c_str(), &end, 10);
    if (errno != ERANGE && end != value.c_str() && *end == '\0') {
        return true;
    }

    if (value.front() == '-') {
        return false;
    }
    errno = 0;
    std::strtoull(value.c_str(), &end, 10);
    return errno != ERANGE && end != value.c_str() && *end == '\0';
}

// Check a whole CSV field, not just a numeric prefix; whitespace stays text.
bool isNumeric(const std::string& value)
{
    if (value.empty() ||
        std::isspace(static_cast<unsigned char>(value.front())) ||
        std::isspace(static_cast<unsigned char>(value.back()))) {
        return false;
    }
    if (isSupportedInteger(value)) {
        return true;
    }

    char* end = nullptr;
    errno = 0;
    std::strtod(value.c_str(), &end);
    return errno != ERANGE && end != value.c_str() && *end == '\0';
}

// Re-quote fields when rebuilding CSV so commas, quotes, and newlines survive.
std::string csvField(const std::string& value)
{
    if (value.find_first_of(",\"\r\n") == std::string::npos) {
        return value;
    }

    std::string escaped{"\""};
    for (const char character : value) {
        if (character == '"') {
            escaped.push_back('"');
        }
        escaped.push_back(character);
    }
    escaped.push_back('"');
    return escaped;
}

// The library deliberately rejects mixed types in a column. Some real-world
// CSVs encode decimal columns with values such as "0" alongside "0.25", so
// this example makes those integer-looking values explicit decimals in memory.
std::string normalizeNumericColumns(const std::string& csv)
{
    // Parse first so quoted commas and embedded line breaks are handled safely.
    datatable::CsvData records = datatable::parseCsv(csv);
    if (records.empty()) {
        return csv;
    }

    // Scan each data column to find numeric columns that contain decimals.
    const std::size_t columnCount = records.front().size();
    std::vector<bool> numeric(columnCount, true);
    std::vector<bool> hasFloatingValue(columnCount, false);

    for (std::size_t rowIndex = 1; rowIndex < records.size(); ++rowIndex) {
        if (records[rowIndex].size() != columnCount) {
            return csv;
        }
        for (std::size_t column = 0; column < columnCount; ++column) {
            const std::string& value = records[rowIndex][column];
            if (value.empty()) {
                continue;
            }
            if (!isNumeric(value)) {
                numeric[column] = false;
            }
            if (value.find_first_of(".eE") != std::string::npos) {
                hasFloatingValue[column] = true;
            }
        }
    }

    // Integer-looking values in decimal columns need an explicit decimal
    // marker because the library intentionally does not promote column types.
    for (std::size_t rowIndex = 1; rowIndex < records.size(); ++rowIndex) {
        for (std::size_t column = 0; column < columnCount; ++column) {
            std::string& value = records[rowIndex][column];
            if (numeric[column] && hasFloatingValue[column] &&
                isSupportedInteger(value)) {
                value += ".0";
            }
        }
    }

    // Rebuild CSV with escaping after the type-compatible normalization.
    std::ostringstream normalized;
    for (const datatable::CsvRecord& record : records) {
        for (std::size_t column = 0; column < record.size(); ++column) {
            if (column != 0) {
                normalized.put(',');
            }
            normalized << csvField(record[column]);
        }
        normalized.put('\n');
    }
    return normalized.str();
}

// Resolve a header by name so commands do not depend on fixed column positions.
std::size_t columnIndex(const datatable::DataTable& table,
                        const std::string& name)
{
    const auto& names = table.columnNames();
    const auto found = std::find(names.begin(), names.end(), name);
    if (found == names.end()) {
        throw std::invalid_argument(
            "Dataset is missing required column: " + name
        );
    }
    return static_cast<std::size_t>(std::distance(names.begin(), found));
}

// Read the complete local file and pass normalized CSV to the library loader.
datatable::DataTable loadTable(const std::string& path)
{
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open input CSV: " + path);
    }

    std::ostringstream contents;
    contents << input.rdbuf();
    if (input.bad()) {
        throw std::ios_base::failure("Failed while reading input CSV.");
    }

    return datatable::DataTable::fromCsv(
        normalizeNumericColumns(contents.str())
    );
}

// Convert the command-line filter value to the same Cell type as its column.
datatable::Cell parseFilterValue(const datatable::Column& column,
                                 const std::string& value)
{
    const std::optional<datatable::CellType> type = column.type();
    if (!type || *type == datatable::CellType::string) {
        return datatable::Cell(value);
    }
    if (*type == datatable::CellType::boolean) {
        if (value == "true") {
            return datatable::Cell(true);
        }
        if (value == "false") {
            return datatable::Cell(false);
        }
        throw std::invalid_argument("Boolean filter values must be true or false.");
    }
    if (*type == datatable::CellType::signed_integer) {
        std::size_t parsed = 0;
        const long long number = std::stoll(value, &parsed);
        if (parsed != value.size()) {
            throw std::invalid_argument("Invalid integer filter value.");
        }
        return datatable::Cell(number);
    }
    if (*type == datatable::CellType::unsigned_integer) {
        std::size_t parsed = 0;
        const unsigned long long number = std::stoull(value, &parsed);
        if (parsed != value.size() || value.front() == '-') {
            throw std::invalid_argument("Invalid unsigned filter value.");
        }
        return datatable::Cell(number);
    }

    std::size_t parsed = 0;
    const double number = std::stod(value, &parsed);
    if (parsed != value.size()) {
        throw std::invalid_argument("Invalid decimal filter value.");
    }
    return datatable::Cell(number);
}

// Select CSV or HTML output based on the requested output file extension.
void saveTable(const datatable::DataTable& table,
               const std::string& path,
               bool html)
{
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Could not open output file: " + path);
    }

    if (html) {
        datatable::write_html(table, output);
    }
    else {
        datatable::write_csv(table, output);
    }
    if (!output) {
        throw std::ios_base::failure("Failed while writing output file.");
    }
}

// Show the supported command shapes when arguments are missing or invalid.
void printUsage()
{
    std::cerr
        << "Usage:\n"
        << "  dataset_example sort <input.csv> <output.csv> [column]\n"
        << "  dataset_example filter <input.csv> <output.csv> <column> <value>\n"
        << "  dataset_example export <input.csv> <output.csv|output.html>\n"
        << "  dataset_example format <input.csv> [rows-to-display] [output.html]\n"
        << "  dataset_example group <input.csv> <column>\n"
        << "  dataset_example summary <day.csv> <output.html>\n";
}

std::string seasonLabel(const datatable::Cell& key)
{
    // Translate the dataset's numeric season codes into report-friendly names.
    const std::string value = key.toString();
    if (value == "1") {
        return "Spring";
    }
    if (value == "2") {
        return "Summer";
    }
    if (value == "3") {
        return "Fall";
    }
    if (value == "4") {
        return "Winter";
    }
    return "Season " + value;
}

double numericValue(const datatable::Cell& cell)
{
    // Convert supported numeric Cell alternatives for display-only formatting.
    if (const std::int64_t* value = cell.getIf<std::int64_t>()) {
        return static_cast<double>(*value);
    }
    if (const std::uint64_t* value = cell.getIf<std::uint64_t>()) {
        return static_cast<double>(*value);
    }
    if (const double* value = cell.getIf<double>()) {
        return *value;
    }
    throw std::invalid_argument("Expected a numeric aggregate value.");
}

std::string fixedNumber(double value, int precision)
{
    // Store a rounded display string so both HTML and terminal output agree.
    std::ostringstream formatted;
    formatted << std::fixed << std::setprecision(precision) << value;
    return formatted.str();
}

datatable::DataTable seasonalSummary(const datatable::DataTable& source)
{
    // Resolve columns by header, then select the dataset's second year (2012).
    const std::size_t yearIndex = columnIndex(source, "yr");
    const std::size_t seasonIndex = columnIndex(source, "season");
    const std::size_t rentalsIndex = columnIndex(source, "cnt");

    const datatable::DataTable filtered =
        datatable::filterEquals(source, yearIndex, datatable::Cell{1});

    // Discover the season keys and their day counts from the filtered rows.
    const datatable::GroupedColumn groups =
        datatable::groupBy(filtered, seasonIndex);

    // Keep aggregate values numeric until after sorting so totals sort by value.
    datatable::DataTable numericSummary;
    numericSummary.addColumn("Season", datatable::CellType::string);
    numericSummary.addColumn("Days", datatable::CellType::unsigned_integer);
    numericSummary.addColumn(
        "Total rentals", datatable::CellType::floating_point
    );
    numericSummary.addColumn(
        "Average rentals", datatable::CellType::floating_point
    );
    numericSummary.addColumn(
        "Minimum rentals", datatable::CellType::floating_point
    );
    numericSummary.addColumn(
        "Maximum rentals", datatable::CellType::floating_point
    );

    for (const datatable::Group& group : groups) {
        // Select one season's rows to calculate its rental statistics.
        const datatable::DataTable seasonRows =
            datatable::filterEquals(filtered, seasonIndex, group.key());
        if (datatable::count(seasonRows, rentalsIndex) == 0) {
            throw std::invalid_argument(
                "A season has no non-null rental counts to aggregate."
            );
        }

        const double average =
            std::round(datatable::mean(seasonRows, rentalsIndex) * 100.0) /
            100.0;
        // The group count is the number of days; aggregates summarize daily cnt.
        numericSummary.addRow(datatable::Row{
            seasonLabel(group.key()),
            static_cast<std::uint64_t>(group.count()),
            datatable::sum(seasonRows, rentalsIndex),
            average,
            numericValue(datatable::min(seasonRows, rentalsIndex)),
            numericValue(datatable::max(seasonRows, rentalsIndex))
        });
    }

    datatable::sortByColumn(
        numericSummary, 2, datatable::SortOrder::descending
    );

    // Convert result cells to display strings after sorting to control decimals.
    datatable::DataTable summary;
    for (const std::string& name : numericSummary.columnNames()) {
        summary.addColumn(name, datatable::CellType::string);
    }
    for (const datatable::Row& row : numericSummary.rows()) {
        summary.addRow(datatable::Row{
            row[0].toString(),
            row[1].toString(),
            fixedNumber(numericValue(row[2]), 0),
            fixedNumber(numericValue(row[3]), 2),
            fixedNumber(numericValue(row[4]), 0),
            fixedNumber(numericValue(row[5]), 0)
        });
    }
    return summary;
}

} // namespace

int main(int argc, char* argv[])
{
    // Validate that the user supplied at least an operation and input path.
    if (argc < 3) {
        printUsage();
        return 2;
    }

    try {
        // Load once, then dispatch to the requested table operation.
        const std::string operation = argv[1];
        datatable::DataTable table = loadTable(argv[2]);

        if (operation == "sort") {
            if (argc != 4 && argc != 5) {
                printUsage();
                return 2;
            }
            const std::string name = argc == 5 ? argv[4] : "cnt";
            // Sort complete rows by the selected key, descending by default.
            datatable::sortByColumn(
                table,
                columnIndex(table, name),
                datatable::SortOrder::descending
            );
            saveTable(table, argv[3], false);
            std::cout << "Sorted " << name << " descending into "
                      << argv[3] << '\n';
        }
        else if (operation == "filter") {
            if (argc != 6) {
                printUsage();
                return 2;
            }
            const std::size_t index = columnIndex(table, argv[4]);
            // Parse the filter value using the column's stored type metadata.
            const datatable::Cell expected =
                parseFilterValue(table.column(index), argv[5]);
            const datatable::DataTable filtered =
                datatable::filterEquals(table, index, expected);
            saveTable(filtered, argv[3], false);
            std::cout << "Matched " << filtered.rowCount() << " rows; wrote "
                      << argv[3] << '\n';
        }
        else if (operation == "export") {
            if (argc != 4) {
                printUsage();
                return 2;
            }
            const std::string path = argv[3];
            // An .html suffix selects HTML; other output names produce CSV.
            const bool html = path.size() >= 5 &&
                path.substr(path.size() - 5) == ".html";
            saveTable(table, path, html);
            std::cout << "Exported " << table.rowCount() << " rows to "
                      << path << '\n';
        }
        else if (operation == "format") {
            if (argc < 3 || argc > 5 || (argc == 5 && argv[4][0] == '\0')) {
                printUsage();
                return 2;
            }
            const std::size_t requested = argc >= 4
                ? static_cast<std::size_t>(std::stoull(argv[3]))
                : 20;
            const std::size_t limit = std::min(requested, table.rowCount());
            // Build a typed preview table so formatting only prints requested
            // rows while preserving the original schema and leaving it intact.
            datatable::DataTable preview;
            for (const datatable::Column& column : table.columns()) {
                if (column.type()) {
                    preview.addColumn(column.name(), *column.type());
                }
                else {
                    preview.addColumn(column.name());
                }
            }
            for (std::size_t index = 0; index < limit; ++index) {
                preview.addRow(table.rows()[index]);
            }
            datatable::Style style;
            // Limit decimal display noise without changing stored values.
            style.setDecimalPrecision(3);
            std::cout << datatable::formatTable(preview, style);
            if (argc == 5) {
                // Export only the preview rows as a browser-ready HTML table.
                saveTable(preview, argv[4], true);
                std::cout << "Exported formatted preview with " << limit
                          << " rows to " << argv[4] << '\n';
            }
            if (limit < table.rowCount()) {
                std::cout << "Displayed " << limit << " of "
                          << table.rowCount() << " rows.\n";
            }
        }
        else if (operation == "group") {
            if (argc != 4) {
                printUsage();
                return 2;
            }
            const std::size_t index = columnIndex(table, argv[3]);
            // Grouping reports each distinct key and its number of rows.
            const datatable::GroupedColumn groups =
                datatable::groupBy(table, index);
            std::cout << "Groups for " << argv[3] << ":\n";
            for (const datatable::Group& group : groups) {
                std::cout << "  " << group.key().toString()
                          << ": " << group.count() << " rows\n";
            }
        }
        else if (operation == "summary") {
            if (argc != 4) {
                printUsage();
                return 2;
            }
            // Export the same analyzed table that is formatted for the console.
            const datatable::DataTable summary = seasonalSummary(table);
            saveTable(summary, argv[3], true);

            datatable::Style style;
            style.setAlignment(datatable::Alignment::center);
            style.setDecimalPrecision(2);
            std::cout << datatable::formatTable(summary, style);
            std::cout << "Filtered to 2012, grouped by season, aggregated "
                         "rental counts, and sorted by total rentals "
                         "descending. Exported "
                      << summary.rowCount() << " groups to " << argv[3]
                      << '\n';
        }
        else {
            printUsage();
            throw std::invalid_argument("Unknown operation: " + operation);
        }
    }
    catch (const std::exception& error) {
        // Report file, parsing, type, and argument errors and return failure.
        std::cerr << "Dataset example failed: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
