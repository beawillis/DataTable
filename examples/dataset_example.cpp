#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
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

std::string normalizeNumericColumns(const std::string& csv)
{
    const datatable::CsvData records = datatable::parseCsv(csv);
    if (records.empty()) {
        return csv;
    }

    const std::size_t columnCount = records.front().size();
    std::vector<bool> numeric(columnCount, true);
    std::vector<bool> hasFloatingValue(columnCount, false);

    for (std::size_t row = 1; row < records.size(); ++row) {
        if (records[row].size() != columnCount) {
            return csv;
        }
        for (std::size_t column = 0; column < columnCount; ++column) {
            const std::string& value = records[row][column];
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

    datatable::CsvData normalizedRecords = records;
    for (std::size_t row = 1; row < normalizedRecords.size(); ++row) {
        for (std::size_t column = 0; column < columnCount; ++column) {
            std::string& value = normalizedRecords[row][column];
            if (numeric[column] && hasFloatingValue[column] &&
                isSupportedInteger(value)) {
                value += ".0";
            }
        }
    }

    std::ostringstream normalized;
    for (const datatable::CsvRecord& record : normalizedRecords) {
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
        throw std::invalid_argument(
            "Boolean filter values must be true or false."
        );
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

void saveCsvTable(const datatable::DataTable& table,
                  const std::string& path)
{
    const std::size_t dot = path.find_last_of('.');
    std::string extension = dot == std::string::npos
        ? std::string{}
        : path.substr(dot);
    std::transform(
        extension.begin(), extension.end(), extension.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        }
    );
    if (extension != ".csv") {
        throw std::invalid_argument(
            "This dataset command writes CSV only; use a .csv output path."
        );
    }

    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Could not open output file: " + path);
    }
    datatable::write_csv(table, output);
    if (!output) {
        throw std::ios_base::failure("Failed while writing CSV output.");
    }
}

void printUsage()
{
    std::cerr
        << "Usage:\n"
        << "  dataset_example sort <input.csv> <output.csv> [column]\n"
        << "  dataset_example filter <input.csv> <output.csv> <column> <value>\n"
        << "  dataset_example export <input.csv> <output.csv>\n"
        << "  dataset_example format <input.csv> [rows-to-display]\n"
        << "  dataset_example group <input.csv> <column>\n"
        << "  dataset_example summary <day.csv> <output.csv>\n"
        << "  dataset_example sza-summary <sza.csv> <output.csv> [latitude]\n";
}

std::string seasonLabel(const datatable::Cell& key)
{
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
    std::ostringstream formatted;
    formatted << std::fixed << std::setprecision(precision) << value;
    return formatted.str();
}

datatable::DataTable seasonalSummary(const datatable::DataTable& source)
{
    const std::size_t yearIndex = columnIndex(source, "yr");
    const std::size_t seasonIndex = columnIndex(source, "season");
    const std::size_t rentalsIndex = columnIndex(source, "cnt");
    const datatable::DataTable filtered =
        datatable::filterEquals(source, yearIndex, datatable::Cell{1});
    const datatable::GroupedColumn groups =
        datatable::groupBy(filtered, seasonIndex);

    datatable::DataTable numericSummary;
    numericSummary.addColumn("Season", datatable::CellType::string);
    numericSummary.addColumn("Days", datatable::CellType::unsigned_integer);
    numericSummary.addColumn("Total rentals", datatable::CellType::floating_point);
    numericSummary.addColumn("Average rentals", datatable::CellType::floating_point);
    numericSummary.addColumn("Minimum rentals", datatable::CellType::floating_point);
    numericSummary.addColumn("Maximum rentals", datatable::CellType::floating_point);

    for (const datatable::Group& group : groups) {
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

datatable::DataTable solarZenithSummary(
    const datatable::DataTable& source,
    const datatable::Cell& latitude
)
{
    const std::size_t latitudeIndex = columnIndex(source, "latitude");
    const std::size_t monthIndex = columnIndex(source, "month");
    const std::size_t angleIndex = columnIndex(source, "sza");
    const datatable::DataTable filtered =
        datatable::filterEquals(source, latitudeIndex, latitude);
    const datatable::GroupedColumn groups =
        datatable::groupBy(filtered, monthIndex);

    datatable::DataTable numericSummary;
    numericSummary.addColumn("Month", datatable::CellType::string);
    numericSummary.addColumn(
        "Valid observations", datatable::CellType::unsigned_integer
    );
    numericSummary.addColumn(
        "Average angle (degrees)", datatable::CellType::floating_point
    );
    numericSummary.addColumn(
        "Minimum angle (degrees)", datatable::CellType::floating_point
    );
    numericSummary.addColumn(
        "Maximum angle (degrees)", datatable::CellType::floating_point
    );

    for (const datatable::Group& group : groups) {
        const datatable::DataTable monthRows =
            datatable::filterEquals(filtered, monthIndex, group.key());
        const std::size_t validCount = datatable::count(monthRows, angleIndex);
        if (validCount == 0) {
            continue;
        }
        numericSummary.addRow(datatable::Row{
            group.key(),
            static_cast<std::uint64_t>(validCount),
            datatable::mean(monthRows, angleIndex),
            numericValue(datatable::min(monthRows, angleIndex)),
            numericValue(datatable::max(monthRows, angleIndex))
        });
    }

    datatable::sortByColumn(
        numericSummary, 2, datatable::SortOrder::descending
    );
    datatable::DataTable summary;
    for (const std::string& name : numericSummary.columnNames()) {
        summary.addColumn(name, datatable::CellType::string);
    }
    for (const datatable::Row& row : numericSummary.rows()) {
        summary.addRow(datatable::Row{
            row[0].toString(),
            row[1].toString(),
            fixedNumber(numericValue(row[2]), 2),
            fixedNumber(numericValue(row[3]), 1),
            fixedNumber(numericValue(row[4]), 1)
        });
    }
    return summary;
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printUsage();
        return 2;
    }

    try {
        const std::string operation = argv[1];
        datatable::DataTable table = loadTable(argv[2]);

        if (operation == "sort") {
            if (argc != 4 && argc != 5) {
                printUsage();
                return 2;
            }
            const std::string name = argc == 5 ? argv[4] : "cnt";
            datatable::sortByColumn(
                table,
                columnIndex(table, name),
                datatable::SortOrder::descending
            );
            saveCsvTable(table, argv[3]);
            std::cout << "Sorted " << name << " descending into "
                      << argv[3] << '\n';
        }
        else if (operation == "filter") {
            if (argc != 6) {
                printUsage();
                return 2;
            }
            const std::size_t index = columnIndex(table, argv[4]);
            const datatable::Cell expected =
                parseFilterValue(table.column(index), argv[5]);
            const datatable::DataTable filtered =
                datatable::filterEquals(table, index, expected);
            saveCsvTable(filtered, argv[3]);
            std::cout << "Matched " << filtered.rowCount() << " rows; wrote "
                      << argv[3] << '\n';
        }
        else if (operation == "export") {
            if (argc != 4) {
                printUsage();
                return 2;
            }
            saveCsvTable(table, argv[3]);
            std::cout << "Exported " << table.rowCount() << " rows to "
                      << argv[3] << '\n';
        }
        else if (operation == "format") {
            if (argc != 3 && argc != 4) {
                printUsage();
                return 2;
            }
            const std::size_t requested = argc == 4
                ? static_cast<std::size_t>(std::stoull(argv[3]))
                : 20;
            const std::size_t limit = std::min(requested, table.rowCount());
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
            style.setDecimalPrecision(3);
            std::cout << datatable::formatTable(preview, style);
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
            const datatable::DataTable summary = seasonalSummary(table);
            saveCsvTable(summary, argv[3]);
            datatable::Style style;
            style.setAlignment(datatable::Alignment::center);
            style.setDecimalPrecision(2);
            std::cout << datatable::formatTable(summary, style);
            std::cout << "Summary CSV written to " << argv[3] << '\n';
        }
        else if (operation == "sza-summary") {
            if (argc != 4 && argc != 5) {
                printUsage();
                return 2;
            }
            const std::size_t latitudeIndex = columnIndex(table, "latitude");
            const datatable::Cell latitude = argc == 5
                ? parseFilterValue(table.column(latitudeIndex), argv[4])
                : datatable::Cell{20.0};
            const datatable::DataTable summary =
                solarZenithSummary(table, latitude);
            if (summary.empty()) {
                throw std::invalid_argument(
                    "No solar-angle observations found for that latitude."
                );
            }
            saveCsvTable(summary, argv[3]);
            datatable::Style style;
            style.setAlignment(datatable::Alignment::center);
            style.setDecimalPrecision(2);
            std::cout << datatable::formatTable(summary, style);
            std::cout << "Solar summary CSV written to " << argv[3] << '\n';
        }
        else {
            printUsage();
            return 2;
        }
    }
    catch (const std::exception& error) {
        std::cerr << "dataset_example: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
