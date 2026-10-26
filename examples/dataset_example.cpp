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
#include <limits>
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
        << "  dataset_example summary <day.csv> <output.html>\n"
        << "  dataset_example sza-summary <sza.csv> <output.html> [latitude]\n"
        << "  dataset_example sza-heatmap <sza.csv> <output.html> [latitude]\n"
        << "  dataset_example bike-heatmap <day.csv|hour.csv> <output.html>\n"
        << "  dataset_example correlation-heatmap <numeric.csv> <output.html>\n";
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

datatable::DataTable solarZenithSummary(
    const datatable::DataTable& source,
    const datatable::Cell& latitude
)
{
    const std::size_t latitudeIndex = columnIndex(source, "latitude");
    const std::size_t monthIndex = columnIndex(source, "month");
    const std::size_t angleIndex = columnIndex(source, "sza");

    // Keep measurements from one latitude; empty angles remain null.
    const datatable::DataTable filtered =
        datatable::filterEquals(source, latitudeIndex, latitude);
    const datatable::GroupedColumn groups =
        datatable::groupBy(filtered, monthIndex);

    // Aggregate each month's valid solar-angle observations.
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

    // Sort using numeric averages before converting them to display text.
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

std::string htmlEscaped(const std::string& text)
{
    std::string escaped;
    for (const char character : text) {
        switch (character) {
        case '&': escaped += "&amp;"; break;
        case '<': escaped += "&lt;"; break;
        case '>': escaped += "&gt;"; break;
        case '"': escaped += "&quot;"; break;
        case '\'': escaped += "&#39;"; break;
        default: escaped.push_back(character); break;
        }
    }
    return escaped;
}

std::string solarAngleColor(double angle)
{
    // Map 0 degrees to warm yellow and 90 degrees to deep purple.
    const double ratio = std::max(0.0, std::min(angle / 90.0, 1.0));
    const int lowColor[] = {244, 201, 93};
    const int highColor[] = {92, 49, 140};
    int channels[3]{};
    for (std::size_t channel = 0; channel < 3; ++channel) {
        channels[channel] = static_cast<int>(
            std::round(lowColor[channel] +
                ratio * (highColor[channel] - lowColor[channel]))
        );
    }

    std::ostringstream color;
    color << '#' << std::hex << std::setfill('0')
          << std::setw(2) << channels[0]
          << std::setw(2) << channels[1]
          << std::setw(2) << channels[2];
    return color.str();
}

void writeSolarHeatmap(const datatable::DataTable& source,
                       const datatable::Cell& latitude,
                       std::ostream& output)
{
    const std::size_t latitudeIndex = columnIndex(source, "latitude");
    const std::size_t monthIndex = columnIndex(source, "month");
    const std::size_t timeIndex = columnIndex(source, "tst");
    const std::size_t angleIndex = columnIndex(source, "sza");

    // Restrict the color scale to one latitude, as in the source example.
    const datatable::DataTable filtered =
        datatable::filterEquals(source, latitudeIndex, latitude);
    if (filtered.empty()) {
        throw std::invalid_argument(
            "No solar-angle measurements found for that latitude."
        );
    }

    struct Measurement {
        std::string month;
        std::string time;
        std::optional<double> angle;
    };
    std::vector<std::string> months;
    std::vector<std::string> times;
    std::vector<Measurement> measurements;
    measurements.reserve(filtered.rowCount());

    // Preserve the first-seen month and time order from the CSV.
    for (const datatable::Row& row : filtered.rows()) {
        const std::string month = row[monthIndex].toString();
        const std::string time = row[timeIndex].toString();
        if (std::find(months.begin(), months.end(), month) == months.end()) {
            months.push_back(month);
        }
        if (std::find(times.begin(), times.end(), time) == times.end()) {
            times.push_back(time);
        }

        std::optional<double> angle;
        if (!row[angleIndex].isNull()) {
            angle = numericValue(row[angleIndex]);
        }
        measurements.push_back({month, time, angle});
    }

    // Reshape the long-form CSV into one row per month and one column per time.
    std::vector<std::vector<std::optional<double>>> cells(
        months.size(), std::vector<std::optional<double>>(times.size())
    );
    for (const Measurement& measurement : measurements) {
        const auto month = std::find(
            months.begin(), months.end(), measurement.month
        );
        const auto time = std::find(
            times.begin(), times.end(), measurement.time
        );
        const std::size_t rowIndex = static_cast<std::size_t>(
            std::distance(months.begin(), month)
        );
        const std::size_t columnIndex = static_cast<std::size_t>(
            std::distance(times.begin(), time)
        );
        cells[rowIndex][columnIndex] = measurement.angle;
    }

    output
        << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
        << "<meta charset=\"UTF-8\">\n"
        << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
        << "<title>Solar Zenith Angle Heatmap</title>\n"
        << "<style>\n"
        << "body{font-family:Segoe UI,Arial,sans-serif;margin:2rem;"
           "color:#263238;background:#f3f6fa}\n"
        << "h1{font-size:1.45rem;margin-bottom:.35rem}\n"
        << ".subtitle{color:#52616b;margin-top:0}\n"
        << ".table-wrap{overflow-x:auto;background:#fff;padding:1rem;"
           "border-radius:8px;box-shadow:0 2px 12px #00000014}\n"
        << "table{border-collapse:collapse;white-space:nowrap;margin:auto}\n"
        << "caption{text-align:left;font-weight:600;padding:.5rem}\n"
        << "th,td{border:1px solid #d9e0e7;padding:.55rem .65rem;"
           "text-align:center}\n"
        << "thead th{background:#174a7e;color:#fff;position:sticky;top:0}\n"
        << "tbody th{background:#edf2f7;text-align:left}\n"
        << "td.value{font-variant-numeric:tabular-nums}\n"
        << ".legend{display:flex;align-items:center;gap:.75rem;"
           "margin:1rem 0;color:#37474f}\n"
        << ".gradient{height:16px;width:240px;border:1px solid #aab4bf;"
           "background:linear-gradient(90deg,#f4c95d,#5c318c)}\n"
        << ".note{font-size:.9rem;color:#52616b}\n"
        << "@media print{body{margin:.5rem}.table-wrap{overflow:visible;"
           "box-shadow:none;padding:0}}\n"
        << "</style>\n</head>\n<body>\n"
        << "<h1>Solar Zenith Angles by Month and True Solar Time</h1>\n"
        << "<p class=\"subtitle\">Latitude "
        << htmlEscaped(latitude.toString())
        << " degrees north. Cell colors encode solar zenith angle from "
           "0 degrees (yellow) to 90 degrees (purple).</p>\n"
        << "<div class=\"legend\"><span>0°</span>"
        << "<div class=\"gradient\" role=\"img\" "
           "aria-label=\"Yellow to purple scale from zero to ninety degrees\"></div>"
        << "<span>90°</span></div>\n"
        << "<div class=\"table-wrap\"><table>\n"
        << "<caption>Solar zenith angle (degrees)</caption>\n"
        << "<thead><tr><th scope=\"col\">Month</th>";
    for (const std::string& time : times) {
        output << "<th scope=\"col\">" << htmlEscaped(time) << "</th>";
    }
    output << "</tr></thead>\n<tbody>\n";

    for (std::size_t rowIndex = 0; rowIndex < months.size(); ++rowIndex) {
        output << "<tr><th scope=\"row\">"
               << htmlEscaped(months[rowIndex]) << "</th>";
        for (const std::optional<double>& angle : cells[rowIndex]) {
            if (!angle) {
                output << "<td aria-label=\"No measurement\"></td>";
                continue;
            }
            output << "<td class=\"value\" style=\"background-color:"
                   << solarAngleColor(*angle)
                   << ";color:" << (*angle > 61.0 ? "#ffffff" : "#263238")
                   << "\" title=\"" << fixedNumber(*angle, 1)
                   << " degrees\">" << fixedNumber(*angle, 1) << "</td>";
        }
        output << "</tr>\n";
    }
    output << "</tbody></table></div>\n"
           << "<p class=\"note\">Empty cells indicate that sunrise had not "
              "occurred by that time. Values are from the supplied solar "
              "zenith angle dataset.</p>\n"
           << "</body>\n</html>\n";

    if (!output) {
        throw std::ios_base::failure("Failed while writing solar heatmap HTML.");
    }
}

std::string sequentialColor(double value, double minimum, double maximum)
{
    const double ratio = maximum > minimum
        ? (value - minimum) / (maximum - minimum)
        : 0.5;
    const int lowColor[] = {255, 242, 174};
    const int highColor[] = {105, 54, 145};
    int channels[3]{};
    for (std::size_t channel = 0; channel < 3; ++channel) {
        channels[channel] = static_cast<int>(std::round(
            lowColor[channel] + ratio * (highColor[channel] - lowColor[channel])
        ));
    }
    std::ostringstream color;
    color << '#' << std::hex << std::setfill('0')
          << std::setw(2) << channels[0]
          << std::setw(2) << channels[1]
          << std::setw(2) << channels[2];
    return color.str();
}

std::string correlationColor(double correlation)
{
    const int neutralColor[] = {247, 247, 247};
    const int negativeColor[] = {49, 105, 165};
    const int positiveColor[] = {203, 24, 29};
    const double ratio = std::min(1.0, std::abs(correlation));
    const int* to = correlation < 0.0 ? negativeColor : positiveColor;
    int channels[3]{};
    for (std::size_t channel = 0; channel < 3; ++channel) {
        channels[channel] = static_cast<int>(std::round(
            neutralColor[channel] +
            ratio * (to[channel] - neutralColor[channel])
        ));
    }
    std::ostringstream color;
    color << '#' << std::hex << std::setfill('0')
          << std::setw(2) << channels[0]
          << std::setw(2) << channels[1]
          << std::setw(2) << channels[2];
    return color.str();
}

void writeBikeHeatmap(const datatable::DataTable& table,
                      std::ostream& output)
{
    const bool hourly = std::find(
        table.columnNames().begin(), table.columnNames().end(), "hr"
    ) != table.columnNames().end();
    const std::string dimensionName = hourly ? "hr" : "mnth";
    const std::string dimensionLabel = hourly
        ? "Hour of day"
        : "Month of year";
    const std::string comparisonName = hourly ? "season" : "yr";
    const std::size_t dimensionIndex = columnIndex(table, dimensionName);
    const std::size_t comparisonIndex = columnIndex(table, comparisonName);
    const std::size_t rentalsIndex = columnIndex(table, "cnt");
    const datatable::GroupedColumn dimensionGroups =
        datatable::groupBy(table, dimensionIndex);
    const datatable::GroupedColumn comparisonGroups =
        datatable::groupBy(table, comparisonIndex);
    if (dimensionGroups.empty() || comparisonGroups.empty()) {
        throw std::invalid_argument("Cannot build a heatmap from an empty table.");
    }

    std::vector<std::vector<double>> totals(
        dimensionGroups.size(),
        std::vector<double>(comparisonGroups.size(), 0.0)
    );
    std::vector<std::vector<std::size_t>> counts(
        dimensionGroups.size(),
        std::vector<std::size_t>(comparisonGroups.size(), 0)
    );
    for (const datatable::Row& row : table.rows()) {
        if (row[rentalsIndex].isNull()) {
            continue;
        }
        const auto dimension = std::find_if(
            dimensionGroups.begin(), dimensionGroups.end(),
            [&row, dimensionIndex](const datatable::Group& group) {
                return group.key().value() == row[dimensionIndex].value();
            }
        );
        const auto comparison = std::find_if(
            comparisonGroups.begin(), comparisonGroups.end(),
            [&row, comparisonIndex](const datatable::Group& group) {
                return group.key().value() == row[comparisonIndex].value();
            }
        );
        if (dimension == dimensionGroups.end() ||
            comparison == comparisonGroups.end()) {
            throw std::logic_error("Could not map a row to its heatmap group.");
        }
        const std::size_t rowIndex = static_cast<std::size_t>(
            std::distance(dimensionGroups.begin(), dimension)
        );
        const std::size_t columnIndex = static_cast<std::size_t>(
            std::distance(comparisonGroups.begin(), comparison)
        );
        totals[rowIndex][columnIndex] += numericValue(row[rentalsIndex]);
        ++counts[rowIndex][columnIndex];
    }

    std::vector<std::vector<std::optional<double>>> averages(
        dimensionGroups.size(),
        std::vector<std::optional<double>>(comparisonGroups.size())
    );
    double minimum = std::numeric_limits<double>::max();
    double maximum = std::numeric_limits<double>::lowest();
    for (std::size_t rowIndex = 0; rowIndex < dimensionGroups.size(); ++rowIndex) {
        for (std::size_t columnIndex = 0;
             columnIndex < comparisonGroups.size(); ++columnIndex) {
            if (counts[rowIndex][columnIndex] == 0) {
                continue;
            }
            const double average = totals[rowIndex][columnIndex] /
                static_cast<double>(counts[rowIndex][columnIndex]);
            averages[rowIndex][columnIndex] = average;
            minimum = std::min(minimum, average);
            maximum = std::max(maximum, average);
        }
    }

    output
        << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
        << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        << "<title>Bike Sharing Rental Heatmap</title><style>"
        << "body{font-family:Segoe UI,Arial,sans-serif;margin:2rem;"
           "color:#263238;background:#f3f6fa}"
        << ".wrap{overflow:auto;background:#fff;padding:1rem;border-radius:8px;"
           "box-shadow:0 2px 12px #00000014}"
        << "table{border-collapse:collapse;white-space:nowrap;margin:auto}"
        << "th,td{border:1px solid #d9e0e7;padding:.55rem .7rem;text-align:center}"
        << "thead th{background:#174a7e;color:#fff;position:sticky;top:0}"
        << "tbody th{background:#edf2f7;text-align:left}"
        << ".legend{display:flex;align-items:center;gap:.7rem;margin:1rem 0}"
        << ".gradient{width:260px;height:16px;border:1px solid #aab4bf;"
           "background:linear-gradient(90deg,#fff2ae,#693691)}"
        << ".note{color:#52616b}"
        << "@media print{body{margin:.5rem}.wrap{overflow:visible;box-shadow:none}}"
        << "</style></head><body><h1>Bike Sharing: Average Rentals Heatmap</h1>"
        << "<p>Average rental count (<code>cnt</code>) by "
        << dimensionLabel << " and "
        << (hourly ? "season." : "year.")
        << "<div class=\"legend\"><span>Lower average</span>"
        << "<div class=\"gradient\" role=\"img\" aria-label=\"Cream to purple "
           "scale from lower to higher average rentals\"></div>"
        << "<span>Higher average</span></div><div class=\"wrap\"><table>"
        << "<thead><tr><th>" << dimensionLabel << "</th>";
    for (const datatable::Group& comparison : comparisonGroups) {
        output << "<th>";
        if (hourly) {
            output << seasonLabel(comparison.key());
        }
        else {
            output << "20" << (numericValue(comparison.key()) == 0.0
                                   ? "11" : "12");
        }
        output << "</th>";
    }
    output << "</tr></thead><tbody>";
    for (std::size_t rowIndex = 0; rowIndex < dimensionGroups.size(); ++rowIndex) {
        output << "<tr><th>";
        if (hourly) {
            output << std::setfill('0') << std::setw(2)
                   << static_cast<int>(numericValue(
                       dimensionGroups[rowIndex].key()
                   )) << ":00";
        }
        else {
            output << htmlEscaped(dimensionGroups[rowIndex].key().toString());
        }
        output << "</th>";
        for (std::size_t columnIndex = 0;
             columnIndex < comparisonGroups.size(); ++columnIndex) {
            if (!averages[rowIndex][columnIndex]) {
                output << "<td aria-label=\"No values\"></td>";
                continue;
            }
            const double value = *averages[rowIndex][columnIndex];
            output << "<td style=\"background-color:"
                   << sequentialColor(value, minimum, maximum)
                   << ";color:" << (value > (minimum + maximum) / 2.0
                                         ? "#ffffff" : "#263238")
                   << "\" title=\"Mean of " << counts[rowIndex][columnIndex]
                   << " records\">" << fixedNumber(value, 0) << "</td>";
        }
        output << "</tr>";
    }
    output << "</tbody></table></div><p class=\"note\">Each cell is the mean "
              "rental count across all records in that category. Color is "
              "scaled to this table's minimum and maximum. Daily data is "
              "compared by year; hourly data is compared by season.</p>"
              "</body></html>";
    if (!output) {
        throw std::ios_base::failure("Failed while writing bike heatmap HTML.");
    }
}

void writeCorrelationHeatmap(const datatable::DataTable& table,
                             std::ostream& output)
{
    const std::size_t columnCount = table.columnCount();
    if (columnCount == 0) {
        throw std::invalid_argument("Cannot correlate a table with no columns.");
    }
    std::vector<std::vector<std::optional<double>>> correlations(
        columnCount, std::vector<std::optional<double>>(columnCount)
    );
    for (std::size_t left = 0; left < columnCount; ++left) {
        for (std::size_t right = 0; right < columnCount; ++right) {
            double sumLeft = 0.0;
            double sumRight = 0.0;
            double sumLeftSquared = 0.0;
            double sumRightSquared = 0.0;
            double sumProduct = 0.0;
            std::size_t pairedCount = 0;
            for (const datatable::Row& row : table.rows()) {
                if (row[left].isNull() || row[right].isNull()) {
                    continue;
                }
                const double x = numericValue(row[left]);
                const double y = numericValue(row[right]);
                sumLeft += x;
                sumRight += y;
                sumLeftSquared += x * x;
                sumRightSquared += y * y;
                sumProduct += x * y;
                ++pairedCount;
            }
            if (pairedCount < 2) {
                continue;
            }
            const double n = static_cast<double>(pairedCount);
            const double numerator = n * sumProduct - sumLeft * sumRight;
            const double varianceLeft =
                n * sumLeftSquared - sumLeft * sumLeft;
            const double varianceRight =
                n * sumRightSquared - sumRight * sumRight;
            const double denominator = std::sqrt(
                varianceLeft * varianceRight
            );
            if (denominator > 0.0) {
                correlations[left][right] =
                    std::max(-1.0, std::min(1.0, numerator / denominator));
            }
        }
    }

    output
        << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
        << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        << "<title>Car Specifications Correlation Heatmap</title><style>"
        << "body{font-family:Segoe UI,Arial,sans-serif;margin:2rem;"
           "color:#263238;background:#f3f6fa}"
        << ".wrap{overflow:auto;background:#fff;padding:1rem;border-radius:8px;"
           "box-shadow:0 2px 12px #00000014;display:inline-block}"
        << "table{border-collapse:collapse;white-space:nowrap}"
        << "th,td{border:1px solid #d9e0e7;padding:.55rem .7rem;text-align:center}"
        << "th{background:#174a7e;color:#fff}"
        << ".legend{display:flex;align-items:center;gap:.7rem;margin:1rem 0}"
        << ".gradient{width:300px;height:16px;border:1px solid #aab4bf;"
           "background:linear-gradient(90deg,#3169a5,#f7f7f7,#cb181d)}"
        << ".note{color:#52616b}"
        << "</style></head><body><h1>Car Specifications: Pearson Correlations</h1>"
        << "<p>Pairwise Pearson correlation across the numeric car fields.</p>"
        << "<div class=\"legend\"><span>-1 negative</span>"
        << "<div class=\"gradient\" role=\"img\" aria-label=\"Blue negative, "
           "white zero, red positive correlation scale\"></div>"
        << "<span>+1 positive</span></div><div class=\"wrap\"><table>"
        << "<thead><tr><th>Field</th>";
    for (const datatable::Column& column : table.columns()) {
        output << "<th>" << htmlEscaped(column.name()) << "</th>";
    }
    output << "</tr></thead><tbody>";
    for (std::size_t rowIndex = 0; rowIndex < columnCount; ++rowIndex) {
        output << "<tr><th>" << htmlEscaped(
            table.columns()[rowIndex].name()
        ) << "</th>";
        for (std::size_t columnIndex = 0;
             columnIndex < columnCount; ++columnIndex) {
            const std::optional<double>& value =
                correlations[rowIndex][columnIndex];
            if (!value) {
                output << "<td></td>";
                continue;
            }
            output << "<td style=\"background-color:"
                   << correlationColor(*value)
                   << ";color:" << (std::abs(*value) > 0.62
                                         ? "#ffffff" : "#263238")
                   << "\">" << fixedNumber(*value, 2) << "</td>";
        }
        output << "</tr>";
    }
    output << "</tbody></table></div><p class=\"note\">Blue indicates negative "
              "correlation, white is near zero, and red indicates positive "
              "correlation. Missing pairs are left blank.</p></body></html>";
    if (!output) {
        throw std::ios_base::failure(
            "Failed while writing correlation heatmap HTML."
        );
    }
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
            saveTable(summary, argv[3], true);

            datatable::Style style;
            style.setAlignment(datatable::Alignment::center);
            style.setDecimalPrecision(2);
            std::cout << datatable::formatTable(summary, style);
            std::cout << "Filtered solar-angle data to latitude "
                      << latitude.toString()
                      << ", grouped by month, aggregated and sorted by "
                         "average angle descending. Exported "
                      << summary.rowCount() << " months to " << argv[3]
                      << '\n';
        }
        else if (operation == "sza-heatmap") {
            if (argc != 4 && argc != 5) {
                printUsage();
                return 2;
            }
            const std::size_t latitudeIndex = columnIndex(table, "latitude");
            const datatable::Cell latitude = argc == 5
                ? parseFilterValue(table.column(latitudeIndex), argv[4])
                : datatable::Cell{20.0};

            std::ofstream output(argv[3], std::ios::binary);
            if (!output) {
                throw std::runtime_error(
                    "Could not open output file: " + std::string(argv[3])
                );
            }
            writeSolarHeatmap(table, latitude, output);
            std::cout << "Exported color-coded solar-angle heatmap for "
                      << "latitude " << latitude.toString() << " to "
                      << argv[3] << '\n';
        }
        else if (operation == "bike-heatmap") {
            if (argc != 4) {
                printUsage();
                return 2;
            }
            std::ofstream output(argv[3], std::ios::binary);
            if (!output) {
                throw std::runtime_error(
                    "Could not open output file: " + std::string(argv[3])
                );
            }
            writeBikeHeatmap(table, output);
            std::cout << "Exported color-coded Bike Sharing "
                      << (std::find(
                              table.columnNames().begin(),
                              table.columnNames().end(),
                              "hr"
                          ) != table.columnNames().end()
                              ? "hourly" : "daily")
                      << " heatmap to " << argv[3] << '\n';
        }
        else if (operation == "correlation-heatmap") {
            if (argc != 4) {
                printUsage();
                return 2;
            }
            std::ofstream output(argv[3], std::ios::binary);
            if (!output) {
                throw std::runtime_error(
                    "Could not open output file: " + std::string(argv[3])
                );
            }
            writeCorrelationHeatmap(table, output);
            std::cout << "Exported Pearson correlation heatmap for "
                      << table.columnCount() << " numeric fields to "
                      << argv[3] << '\n';
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
