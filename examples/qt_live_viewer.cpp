#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <QAbstractTableModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QDir>
#include <QFont>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPen>
#include <QPixmap>
#include <QTableView>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "datatable/csv.hpp"
#include "datatable/row.hpp"
#include "datatable/table.hpp"

namespace {

std::optional<double> numericValue(const datatable::Cell& cell)
{
    if (cell.isNull()) {
        return std::nullopt;
    }

    const std::string text = cell.toString();
    char* end = nullptr;
    errno = 0;
    const double value = std::strtod(text.c_str(), &end);
    if (errno == ERANGE || end == text.c_str() || *end != '\0' ||
        !std::isfinite(value)) {
        return std::nullopt;
    }
    return value;
}

std::size_t columnIndex(const datatable::DataTable& table,
                        const std::string& name)
{
    const auto& names = table.columnNames();
    const auto found = std::find(names.begin(), names.end(), name);
    if (found == names.end()) {
        throw std::invalid_argument("Dataset is missing column: " + name);
    }
    return static_cast<std::size_t>(std::distance(names.begin(), found));
}

QString cellText(const datatable::Cell& cell)
{
    return cell.isNull()
        ? QString{}
        : QString::fromStdString(cell.toString());
}

QString seasonName(const datatable::Cell& cell)
{
    const QString code = cellText(cell);
    if (code == "1") {
        return "Spring";
    }
    if (code == "2") {
        return "Summer";
    }
    if (code == "3") {
        return "Fall";
    }
    if (code == "4") {
        return "Winter";
    }
    return "Season " + code;
}

QColor sequentialColor(double value, double minimum, double maximum)
{
    const double ratio = maximum > minimum
        ? std::max(0.0, std::min((value - minimum) / (maximum - minimum), 1.0))
        : 0.5;
    const QColor low(255, 242, 174);
    const QColor high(105, 54, 145);
    return QColor(
        static_cast<int>(low.red() + ratio * (high.red() - low.red())),
        static_cast<int>(low.green() + ratio * (high.green() - low.green())),
        static_cast<int>(low.blue() + ratio * (high.blue() - low.blue()))
    );
}

QColor divergingColor(double value)
{
    const QColor neutral(247, 247, 247);
    const QColor endpoint = value < 0.0
        ? QColor(49, 105, 165)
        : QColor(203, 24, 29);
    const double ratio = std::min(std::abs(value), 1.0);
    return QColor(
        static_cast<int>(neutral.red() + ratio * (endpoint.red() - neutral.red())),
        static_cast<int>(neutral.green() + ratio * (endpoint.green() - neutral.green())),
        static_cast<int>(neutral.blue() + ratio * (endpoint.blue() - neutral.blue()))
    );
}

struct NativeVisual
{
    QString title;
    QString subtitle;
    QString lowLabel;
    QString highLabel;
    std::vector<QString> headers;
    std::vector<QString> rowLabels;
    std::vector<std::vector<QString>> text;
    std::vector<std::vector<std::optional<double>>> colors;
    double minimum = 0.0;
    double maximum = 0.0;
    bool diverging = false;
    bool showLegend = true;
    bool summaryTable = false;
};

std::vector<std::string> orderedCategories(
    const datatable::DataTable& table,
    std::size_t column,
    const std::vector<std::string>& prescribedOrder = {})
{
    std::vector<std::string> values;
    for (const datatable::Row& row : table.rows()) {
        if (row[column].isNull()) {
            continue;
        }
        const std::string value = row[column].toString();
        if (std::find(values.begin(), values.end(), value) == values.end()) {
            values.push_back(value);
        }
    }

    if (!prescribedOrder.empty()) {
        std::stable_sort(values.begin(), values.end(),
            [&prescribedOrder](const std::string& left,
                               const std::string& right) {
                const auto leftPosition = std::find(
                    prescribedOrder.begin(), prescribedOrder.end(), left
                );
                const auto rightPosition = std::find(
                    prescribedOrder.begin(), prescribedOrder.end(), right
                );
                return leftPosition < rightPosition;
            });
        return values;
    }

    const bool allNumeric = std::all_of(
        values.begin(), values.end(),
        [](const std::string& value) {
            char* end = nullptr;
            std::strtod(value.c_str(), &end);
            return end != value.c_str() && *end == '\0';
        }
    );
    std::sort(values.begin(), values.end(),
        [allNumeric](const std::string& left, const std::string& right) {
            if (allNumeric) {
                return std::strtod(left.c_str(), nullptr) <
                    std::strtod(right.c_str(), nullptr);
            }
            return left < right;
        });
    return values;
}

NativeVisual makeBikeVisual(const datatable::DataTable& table,
                            bool hourly,
                            bool secondHalf)
{
    const std::size_t dimensionIndex = columnIndex(table, hourly ? "hr" : "mnth");
    const std::size_t comparisonIndex =
        columnIndex(table, hourly ? "season" : "yr");
    const std::size_t rentalsIndex = columnIndex(table, "cnt");
    const std::vector<std::string> dimensions =
        orderedCategories(table, dimensionIndex);
    const std::vector<std::string> comparisons =
        orderedCategories(table, comparisonIndex);
    if (dimensions.empty() || comparisons.empty()) {
        throw std::invalid_argument("Cannot create a heatmap from an empty dataset.");
    }

    const std::size_t split = (dimensions.size() + 1) / 2;
    const std::size_t begin = secondHalf ? split : 0;
    const std::size_t end = secondHalf ? dimensions.size() : split;

    std::map<std::string, std::size_t> dimensionPositions;
    std::map<std::string, std::size_t> comparisonPositions;
    for (std::size_t index = 0; index < dimensions.size(); ++index) {
        dimensionPositions.emplace(dimensions[index], index);
    }
    for (std::size_t index = 0; index < comparisons.size(); ++index) {
        comparisonPositions.emplace(comparisons[index], index);
    }

    std::vector<std::vector<double>> totals(
        dimensions.size(), std::vector<double>(comparisons.size(), 0.0)
    );
    std::vector<std::vector<std::size_t>> counts(
        dimensions.size(), std::vector<std::size_t>(comparisons.size(), 0)
    );
    for (const datatable::Row& row : table.rows()) {
        const std::optional<double> rentalCount = numericValue(row[rentalsIndex]);
        if (!rentalCount) {
            continue;
        }
        const auto dimension =
            dimensionPositions.find(row[dimensionIndex].toString());
        const auto comparison =
            comparisonPositions.find(row[comparisonIndex].toString());
        if (dimension == dimensionPositions.end() ||
            comparison == comparisonPositions.end()) {
            continue;
        }
        totals[dimension->second][comparison->second] += *rentalCount;
        ++counts[dimension->second][comparison->second];
    }

    std::vector<std::vector<std::optional<double>>> averages(
        dimensions.size(),
        std::vector<std::optional<double>>(comparisons.size())
    );
    double minimum = std::numeric_limits<double>::max();
    double maximum = std::numeric_limits<double>::lowest();
    for (std::size_t row = 0; row < dimensions.size(); ++row) {
        for (std::size_t column = 0; column < comparisons.size(); ++column) {
            if (counts[row][column] == 0) {
                continue;
            }
            const double average =
                totals[row][column] / static_cast<double>(counts[row][column]);
            averages[row][column] = average;
            minimum = std::min(minimum, average);
            maximum = std::max(maximum, average);
        }
    }

    NativeVisual visual;
    visual.title = "Bike Sharing: Average Rentals Heatmap";
    visual.subtitle = QString("Average rental count (cnt) by %1 and %2.")
        .arg(hourly ? "hour of day" : "month of year")
        .arg(hourly ? "season" : "year");
    visual.lowLabel = "Lower average";
    visual.highLabel = "Higher average";
    visual.headers.push_back(hourly ? "Hour of day" : "Month of year");
    for (const std::string& comparison : comparisons) {
        if (hourly) {
            visual.headers.push_back(
                seasonName(datatable::Cell(comparison))
            );
        }
        else {
            visual.headers.push_back(
                std::strtod(comparison.c_str(), nullptr) == 0.0
                    ? "2011" : "2012"
            );
        }
    }
    for (std::size_t row = begin; row < end; ++row) {
        QString label = QString::fromStdString(dimensions[row]);
        if (hourly) {
            label = QString("%1:00").arg(
                static_cast<int>(std::strtod(
                    dimensions[row].c_str(), nullptr
                )),
                2, 10, QChar('0')
            );
        }
        visual.rowLabels.push_back(label);

        std::vector<QString> cells;
        std::vector<std::optional<double>> colors;
        for (std::size_t column = 0; column < comparisons.size(); ++column) {
            if (!averages[row][column]) {
                cells.emplace_back();
                colors.emplace_back();
                continue;
            }
            cells.push_back(QString::number(*averages[row][column], 'f', 0));
            colors.push_back(averages[row][column]);
        }
        visual.text.push_back(std::move(cells));
        visual.colors.push_back(std::move(colors));
    }
    visual.minimum = minimum;
    visual.maximum = maximum;
    return visual;
}

NativeVisual makeSolarVisual(const datatable::DataTable& table,
                             bool secondHalf)
{
    const std::size_t latitudeIndex = columnIndex(table, "latitude");
    const std::size_t monthIndex = columnIndex(table, "month");
    const std::size_t timeIndex = columnIndex(table, "tst");
    const std::size_t angleIndex = columnIndex(table, "sza");

    std::vector<std::string> months = orderedCategories(
        table, monthIndex,
        {"jan", "feb", "mar", "apr", "may", "jun",
         "jul", "aug", "sep", "oct", "nov", "dec"}
    );
    std::vector<std::string> times = orderedCategories(table, timeIndex);
    const double latitude = 20.0;
    const auto latitudeRow = std::find_if(
        table.rows().begin(), table.rows().end(),
        [latitudeIndex, latitude](const datatable::Row& row) {
            const auto value = numericValue(row[latitudeIndex]);
            return value && *value == latitude;
        }
    );
    if (latitudeRow == table.rows().end()) {
        throw std::invalid_argument("Solar dataset has no latitude 20 values.");
    }

    std::vector<std::vector<std::optional<double>>> values(
        months.size(), std::vector<std::optional<double>>(times.size())
    );
    for (const datatable::Row& row : table.rows()) {
        const auto rowLatitude = numericValue(row[latitudeIndex]);
        if (!rowLatitude || *rowLatitude != latitude) {
            continue;
        }
        const auto month = std::find(months.begin(), months.end(),
                                     row[monthIndex].toString());
        const auto time = std::find(times.begin(), times.end(),
                                    row[timeIndex].toString());
        const auto angle = numericValue(row[angleIndex]);
        if (month == months.end() || time == times.end() || !angle) {
            continue;
        }
        values[static_cast<std::size_t>(std::distance(months.begin(), month))]
              [static_cast<std::size_t>(std::distance(times.begin(), time))] =
            angle;
    }

    const std::size_t split = (months.size() + 1) / 2;
    const std::size_t begin = secondHalf ? split : 0;
    const std::size_t end = secondHalf ? months.size() : split;

    NativeVisual visual;
    visual.title = "Solar Zenith Angles by Month and True Solar Time";
    visual.subtitle =
        "Latitude 20 degrees north. Colors encode angles from 0 to 90 degrees.";
    visual.lowLabel = "0 degrees";
    visual.highLabel = "90 degrees";
    visual.headers.push_back("Month");
    for (const std::string& time : times) {
        visual.headers.push_back(QString::fromStdString(time));
    }
    for (std::size_t row = begin; row < end; ++row) {
        visual.rowLabels.push_back(QString::fromStdString(months[row]));
        std::vector<QString> cells;
        std::vector<std::optional<double>> colors;
        for (std::size_t column = 0; column < times.size(); ++column) {
            if (!values[row][column]) {
                cells.emplace_back();
                colors.emplace_back();
            }
            else {
                cells.push_back(QString::number(*values[row][column], 'f', 1));
                colors.push_back(values[row][column]);
            }
        }
        visual.text.push_back(std::move(cells));
        visual.colors.push_back(std::move(colors));
    }
    visual.minimum = 0.0;
    visual.maximum = 90.0;
    return visual;
}

NativeVisual makeCorrelationVisual(const datatable::DataTable& table)
{
    NativeVisual visual;
    visual.title = "Car Specifications: Pearson Correlations";
    visual.subtitle = "Pairwise Pearson correlation across the numeric car fields.";
    visual.lowLabel = "-1 negative";
    visual.highLabel = "+1 positive";
    visual.diverging = true;
    visual.minimum = -1.0;
    visual.maximum = 1.0;
    visual.headers.push_back("Field");
    for (const std::string& name : table.columnNames()) {
        visual.headers.push_back(QString::fromStdString(name));
        visual.rowLabels.push_back(QString::fromStdString(name));
    }

    const std::size_t count = table.columnCount();
    for (std::size_t left = 0; left < count; ++left) {
        std::vector<QString> cells;
        std::vector<std::optional<double>> colors;
        for (std::size_t right = 0; right < count; ++right) {
            double sumX = 0.0;
            double sumY = 0.0;
            double sumXX = 0.0;
            double sumYY = 0.0;
            double sumXY = 0.0;
            std::size_t paired = 0;
            for (const datatable::Row& row : table.rows()) {
                const auto x = numericValue(row[left]);
                const auto y = numericValue(row[right]);
                if (!x || !y) {
                    continue;
                }
                sumX += *x;
                sumY += *y;
                sumXX += *x * *x;
                sumYY += *y * *y;
                sumXY += *x * *y;
                ++paired;
            }
            const double n = static_cast<double>(paired);
            const double numerator = n * sumXY - sumX * sumY;
            const double denominator = std::sqrt(
                (n * sumXX - sumX * sumX) *
                (n * sumYY - sumY * sumY)
            );
            if (paired < 2 || denominator == 0.0) {
                cells.emplace_back();
                colors.emplace_back();
                continue;
            }
            const double correlation = numerator / denominator;
            cells.push_back(QString::number(correlation, 'f', 2));
            colors.push_back(correlation);
        }
        visual.text.push_back(std::move(cells));
        visual.colors.push_back(std::move(colors));
    }
    return visual;
}

NativeVisual makeSummaryVisual(const datatable::DataTable& table)
{
    const std::size_t yearIndex = columnIndex(table, "yr");
    const std::size_t seasonIndex = columnIndex(table, "season");
    const std::size_t rentalsIndex = columnIndex(table, "cnt");
    struct Summary {
        std::string season;
        std::size_t days = 0;
        double total = 0.0;
        double minimum = std::numeric_limits<double>::max();
        double maximum = std::numeric_limits<double>::lowest();
    };
    std::map<int, Summary> summaries;
    for (const datatable::Row& row : table.rows()) {
        const auto year = numericValue(row[yearIndex]);
        const auto season = numericValue(row[seasonIndex]);
        const auto rentals = numericValue(row[rentalsIndex]);
        if (!year || *year != 1.0 || !season || !rentals) {
            continue;
        }
        Summary& summary = summaries[static_cast<int>(*season)];
        summary.season = seasonName(row[seasonIndex]).toStdString();
        ++summary.days;
        summary.total += *rentals;
        summary.minimum = std::min(summary.minimum, *rentals);
        summary.maximum = std::max(summary.maximum, *rentals);
    }
    std::vector<Summary> ordered;
    for (const auto& entry : summaries) {
        ordered.push_back(entry.second);
    }
    std::sort(ordered.begin(), ordered.end(),
        [](const Summary& left, const Summary& right) {
            return left.total > right.total;
        });

    NativeVisual visual;
    visual.title = "Bike Sharing: Seasonal Rental Summary";
    visual.subtitle =
        "2012 daily rentals grouped by season and sorted by total rentals.";
    visual.showLegend = false;
    visual.summaryTable = true;
    visual.headers = {
        "Season", "Days", "Total rentals", "Average rentals",
        "Minimum rentals", "Maximum rentals"
    };
    for (const Summary& summary : ordered) {
        visual.rowLabels.push_back(QString::fromStdString(summary.season));
        visual.text.push_back({
            QString::number(summary.days),
            QString::number(summary.total, 'f', 0),
            QString::number(summary.total / static_cast<double>(summary.days),
                            'f', 2),
            QString::number(summary.minimum, 'f', 0),
            QString::number(summary.maximum, 'f', 0)
        });
        visual.colors.emplace_back(5);
    }
    return visual;
}

NativeVisual makeFormattedVisual(const datatable::DataTable& table,
                                bool secondHalf)
{
    const std::size_t begin = secondHalf ? 10 : 0;
    const std::size_t end = std::min(table.rowCount(), begin + 10);
    if (begin >= table.rowCount()) {
        throw std::invalid_argument(
            "The Bike Sharing daily dataset does not have a second preview page."
        );
    }

    NativeVisual visual;
    visual.title = secondHalf
        ? "Bike Sharing Daily Table (continued)"
        : "Bike Sharing Daily Table (first rows)";
    visual.subtitle = QString("Rows %1 through %2 of %3.")
        .arg(static_cast<qulonglong>(begin + 1))
        .arg(static_cast<qulonglong>(end))
        .arg(static_cast<qulonglong>(table.rowCount()));
    visual.showLegend = false;
    visual.summaryTable = true;
    visual.headers.push_back("#");
    for (const std::string& name : table.columnNames()) {
        visual.headers.push_back(QString::fromStdString(name));
    }
    for (std::size_t row = begin; row < end; ++row) {
        visual.rowLabels.push_back(
            QString::number(static_cast<qulonglong>(row + 1))
        );
        std::vector<QString> cells;
        cells.reserve(table.columnCount());
        for (const datatable::Cell& cell : table.rows()[row].cells()) {
            cells.push_back(cellText(cell));
        }
        visual.text.push_back(std::move(cells));
        visual.colors.emplace_back(table.columnCount());
    }
    return visual;
}

class ColorScaleLegend final : public QWidget
{
public:
    explicit ColorScaleLegend(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(38);
        setMaximumHeight(38);
    }

    void setLabels(const QString& low,
                   const QString& high,
                   bool diverging = false)
    {
        lowLabel_ = low;
        highLabel_ = high;
        diverging_ = diverging;
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRectF bar(130.0, 10.0, 300.0, 16.0);
        QLinearGradient gradient(bar.topLeft(), bar.topRight());
        if (diverging_) {
            gradient.setColorAt(0.0, QColor(49, 105, 165));
            gradient.setColorAt(0.5, QColor(247, 247, 247));
            gradient.setColorAt(1.0, QColor(203, 24, 29));
        }
        else {
            gradient.setColorAt(0.0, QColor(255, 242, 174));
            gradient.setColorAt(1.0, QColor(105, 54, 145));
        }
        painter.setPen(QPen(QColor("#aab4bf"), 1.0));
        painter.setBrush(gradient);
        painter.drawRoundedRect(bar, 3.0, 3.0);

        painter.setPen(QColor("#263238"));
        painter.drawText(
            QRectF(0.0, 8.0, 122.0, 20.0),
            Qt::AlignRight | Qt::AlignVCenter,
            lowLabel_.isEmpty() ? "Lower value" : lowLabel_
        );
        painter.drawText(
            QRectF(438.0, 8.0, 160.0, 20.0),
            Qt::AlignLeft | Qt::AlignVCenter,
            highLabel_.isEmpty() ? "Higher value" : highLabel_
        );
    }

private:
    QString lowLabel_;
    QString highLabel_;
    bool diverging_ = false;
};

class NativeVisualViewer final : public QWidget
{
public:
    explicit NativeVisualViewer(const NativeVisual& visual,
                                QWidget* parent = nullptr)
        : QWidget(parent),
          visual_(visual)
    {
        setStyleSheet(
            "QWidget { background: #f3f6fa; color: #263238; }"
            "QLabel#visualTitle { font-size: 22px; font-weight: 700; }"
            "QLabel#visualSubtitle { font-size: 14px; color: #52616b; }"
            "QTableWidget { background: white; gridline-color: #d9e0e7; "
            "border: 1px solid #d9e0e7; }"
            "QHeaderView::section { background: #174a7e; color: white; "
            "font-weight: 700; padding: 9px; border: 1px solid #d9e0e7; }"
        );

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(26, 22, 26, 22);
        layout->setSpacing(10);

        auto* title = new QLabel(visual_.title, this);
        title->setObjectName("visualTitle");
        layout->addWidget(title);

        auto* subtitle = new QLabel(visual_.subtitle, this);
        subtitle->setObjectName("visualSubtitle");
        layout->addWidget(subtitle);

        if (visual_.showLegend) {
            auto* legend = new ColorScaleLegend(this);
            legend->setLabels(
                visual_.lowLabel, visual_.highLabel, visual_.diverging
            );
            layout->addWidget(legend);
        }

        table_ = new QTableWidget(
            static_cast<int>(visual_.rowLabels.size()),
            static_cast<int>(visual_.headers.size()),
            this
        );
        QStringList headers;
        for (const QString& header : visual_.headers) {
            headers.push_back(header);
        }
        table_->setHorizontalHeaderLabels(headers);
        table_->verticalHeader()->hide();
        table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table_->setSelectionMode(QAbstractItemView::NoSelection);
        table_->setShowGrid(true);
        table_->setWordWrap(false);
        table_->horizontalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents
        );
        table_->verticalHeader()->setDefaultSectionSize(30);

        for (std::size_t row = 0; row < visual_.rowLabels.size(); ++row) {
            auto* label = new QTableWidgetItem(visual_.rowLabels[row]);
            label->setFont(QFont("Segoe UI", 10, QFont::Bold));
            label->setBackground(QColor("#edf2f7"));
            label->setForeground(QColor("#263746"));
            table_->setItem(static_cast<int>(row), 0, label);

            for (std::size_t column = 0;
                 column < visual_.text[row].size(); ++column) {
                auto* item = new QTableWidgetItem(visual_.text[row][column]);
                item->setTextAlignment(Qt::AlignCenter);
                item->setFont(QFont("Segoe UI", 10));
                if (!visual_.summaryTable &&
                    row < visual_.colors.size() &&
                    column < visual_.colors[row].size() &&
                    visual_.colors[row][column]) {
                    const double value = *visual_.colors[row][column];
                    const QColor color = visual_.diverging
                        ? divergingColor(value)
                        : sequentialColor(
                            value, visual_.minimum, visual_.maximum
                        );
                    item->setBackground(color);
                    item->setForeground(
                        color.lightness() < 145
                            ? QColor("#ffffff")
                            : QColor("#263238")
                    );
                }
                else if (visual_.summaryTable && row % 2 == 1) {
                    item->setBackground(QColor("#edf3f8"));
                }
                table_->setItem(
                    static_cast<int>(row),
                    static_cast<int>(column + 1),
                    item
                );
            }
            table_->setRowHeight(static_cast<int>(row), 30);
        }
        layout->addWidget(table_);

        if (visual_.diverging) {
            auto* note = new QLabel(
                "Blue indicates negative correlation, white is near zero, "
                "and red indicates positive correlation.",
                this
            );
            note->setObjectName("visualSubtitle");
            layout->addWidget(note);
        }

        table_->resizeColumnsToContents();
        int tableWidth = table_->verticalHeader()->width() + 4;
        for (int column = 0; column < table_->columnCount(); ++column) {
            tableWidth += table_->columnWidth(column);
        }
        const int desiredWidth = std::max(900, tableWidth + 58);
        const int desiredHeight = 260 + table_->rowCount() * 30;
        resize(desiredWidth, std::min(desiredHeight, 1000));
    }

private:
    const NativeVisual& visual_;
    QTableWidget* table_ = nullptr;
};

datatable::DataTable loadCsvTable(const std::string& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Could not open CSV file: " + path);
    }

    const std::string contents{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
    if (input.bad()) {
        throw std::ios_base::failure("Failed while reading CSV file: " + path);
    }

    const datatable::CsvData records = datatable::parseCsv(contents);
    if (records.empty() || records.front().empty()) {
        throw std::invalid_argument("CSV file must include a header row.");
    }

    datatable::DataTable table;
    for (const std::string& name : records.front()) {
        table.addColumn(name, datatable::CellType::string);
    }
    for (std::size_t row = 1; row < records.size(); ++row) {
        if (records[row].size() != records.front().size()) {
            throw std::invalid_argument(
                "CSV row " + std::to_string(row + 1) +
                " has a different number of fields than the header."
            );
        }

        std::vector<datatable::Cell> values;
        for (const std::string& value : records[row]) {
            if (value.empty()) {
                values.push_back(datatable::Cell{});
            }
            else {
                values.push_back(datatable::Cell(value));
            }
        }
        table.addRow(datatable::Row(std::move(values)));
    }
    return table;
}

class DatasetModel final : public QAbstractTableModel
{
public:
    DatasetModel(const datatable::DataTable& table, QObject* parent = nullptr)
        : QAbstractTableModel(parent),
          table_(table),
          ranges_(table.columnCount()),
          numericValues_(
              table.rowCount(),
              std::vector<std::optional<double>>(table.columnCount())
          )
    {
        for (std::size_t row = 0; row < table_.rowCount(); ++row) {
            for (std::size_t column = 0;
                 column < table_.columnCount(); ++column) {
                const std::optional<double> value =
                    numericValue(table_.rows()[row][column]);
                if (!value) {
                    continue;
                }
                numericValues_[row][column] = value;
                if (!ranges_[column]) {
                    ranges_[column] = std::make_pair(*value, *value);
                }
                else {
                    ranges_[column]->first =
                        std::min(ranges_[column]->first, *value);
                    ranges_[column]->second =
                        std::max(ranges_[column]->second, *value);
                }
            }
        }
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : static_cast<int>(table_.rowCount());
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : static_cast<int>(table_.columnCount());
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid()) {
            return {};
        }

        const datatable::Cell& cell = table_.rows()[
            static_cast<std::size_t>(index.row())
        ][static_cast<std::size_t>(index.column())];
        if (role == Qt::DisplayRole) {
            return cell.isNull()
                ? QVariant{}
                : QVariant(QString::fromStdString(cell.toString()));
        }
        if (role != Qt::BackgroundRole && role != Qt::ForegroundRole) {
            return {};
        }

        const std::size_t column = static_cast<std::size_t>(index.column());
        const std::optional<double>& value = numericValues_[
            static_cast<std::size_t>(index.row())
        ][column];
        if (!value || !ranges_[column]) {
            return {};
        }

        const double range = ranges_[column]->second - ranges_[column]->first;
        const double fraction = range > 0.0
            ? (*value - ranges_[column]->first) / range
            : 0.5;
        const QColor low(255, 242, 174);
        const QColor high(105, 54, 145);
        const QColor background(
            static_cast<int>(low.red() + fraction * (high.red() - low.red())),
            static_cast<int>(low.green() + fraction * (high.green() - low.green())),
            static_cast<int>(low.blue() + fraction * (high.blue() - low.blue()))
        );
        if (role == Qt::BackgroundRole) {
            return QBrush(background);
        }
        return QBrush(fraction > 0.62 ? QColor("#ffffff")
                                      : QColor("#263238"));
    }

    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role) const override
    {
        if (role != Qt::DisplayRole) {
            return {};
        }
        if (orientation == Qt::Horizontal) {
            return QString::fromStdString(
                table_.columnNames()[static_cast<std::size_t>(section)]
            );
        }
        return section + 1;
    }

private:
    const datatable::DataTable& table_;
    std::vector<std::optional<std::pair<double, double>>> ranges_;
    std::vector<std::vector<std::optional<double>>> numericValues_;
};

class DatasetViewer final : public QWidget
{
public:
    explicit DatasetViewer(const std::string& path, QWidget* parent = nullptr)
        : QWidget(parent),
          table_(loadCsvTable(path))
    {
        auto* layout = new QVBoxLayout(this);
        auto* description = new QLabel(
            QString("%1 rows  |  %2 columns  |  numeric cells are colored "
                    "from lower to higher values")
                .arg(static_cast<qulonglong>(table_.rowCount()))
                .arg(static_cast<qulonglong>(table_.columnCount())),
            this
        );
        layout->addWidget(description);
        layout->addWidget(new ColorScaleLegend(this));

        auto* view = new QTableView(this);
        view->setModel(new DatasetModel(table_, view));
        view->setShowGrid(true);
        view->setGridStyle(Qt::SolidLine);
        view->setAlternatingRowColors(false);
        view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        view->horizontalHeader()->setDefaultSectionSize(125);
        view->horizontalHeader()->setMinimumSectionSize(80);
        view->verticalHeader()->setDefaultSectionSize(25);
        view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        layout->addWidget(view);

        setWindowTitle(
            QString("DataTable Qt Viewer - %1")
                .arg(QString::fromStdString(path))
        );
        resize(1200, 700);
    }

private:
    datatable::DataTable table_;
};

constexpr int timerIntervalMs = 100;
constexpr std::size_t maximumSamples = 120;
constexpr double sampleIntervalSeconds = 0.1;
constexpr double visibleWindowSeconds = 10.0;
constexpr double minimumValue = -12.0;
constexpr double maximumValue = 12.0;

class LivePlot final : public QWidget
{
public:
    explicit LivePlot(const datatable::DataTable& table, QWidget* parent = nullptr)
        : QWidget(parent),
          table_(table)
    {
        setMinimumSize(420, 300);
        setAutoFillBackground(true);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor("#ffffff"));

        const QRectF plotArea(62.0, 24.0, width() - 86.0, height() - 68.0);
        if (plotArea.width() <= 0.0 || plotArea.height() <= 0.0) {
            return;
        }

        painter.setPen(QPen(QColor("#dce3eb"), 1.0));
        QFont labelFont = painter.font();
        labelFont.setPointSize(9);
        painter.setFont(labelFont);

        for (int tick = 0; tick <= 4; ++tick) {
            const double fraction = static_cast<double>(tick) / 4.0;
            const double y = plotArea.bottom() -
                (fraction * plotArea.height());
            const double value = minimumValue +
                (fraction * (maximumValue - minimumValue));
            painter.drawLine(
                QPointF(plotArea.left(), y),
                QPointF(plotArea.right(), y)
            );
            painter.setPen(QColor("#526173"));
            painter.drawText(
                QRectF(4.0, y - 10.0, 50.0, 20.0),
                Qt::AlignRight | Qt::AlignVCenter,
                QString::number(value, 'f', 0)
            );
            painter.setPen(QPen(QColor("#dce3eb"), 1.0));
        }

        const std::size_t sampleCount = table_.rowCount();
        if (sampleCount == 0) {
            painter.setPen(QColor("#526173"));
            painter.drawText(plotArea, Qt::AlignCenter, "Waiting for samples...");
            return;
        }

        const double latestTime =
            table_.column(0).at(sampleCount - 1).get<double>();
        const double startTime =
            std::max(0.0, latestTime - visibleWindowSeconds);
        const double timeRange = std::max(
            latestTime - startTime,
            sampleIntervalSeconds
        );

        painter.setPen(QPen(QColor("#dce3eb"), 1.0));
        for (int tick = 0; tick <= 5; ++tick) {
            const double fraction = static_cast<double>(tick) / 5.0;
            const double x = plotArea.left() +
                (fraction * plotArea.width());
            const double time = startTime + (fraction * timeRange);
            painter.drawLine(
                QPointF(x, plotArea.top()),
                QPointF(x, plotArea.bottom())
            );
            painter.setPen(QColor("#526173"));
            painter.drawText(
                QRectF(x - 34.0, plotArea.bottom() + 8.0, 68.0, 20.0),
                Qt::AlignHCenter | Qt::AlignTop,
                QString::number(time, 'f', 1)
            );
            painter.setPen(QPen(QColor("#dce3eb"), 1.0));
        }

        painter.setPen(QPen(QColor("#138a83"), 2.5));
        QPainterPath line;
        bool started = false;
        for (std::size_t row = 0; row < sampleCount; ++row) {
            const double time = table_.column(0).at(row).get<double>();
            const double value = table_.column(1).at(row).get<double>();
            if (time < startTime) {
                continue;
            }

            const double x = plotArea.left() +
                ((time - startTime) / timeRange) * plotArea.width();
            const double y = plotArea.bottom() -
                ((value - minimumValue) /
                 (maximumValue - minimumValue)) * plotArea.height();
            const QPointF point(x, y);
            if (!started) {
                line.moveTo(point);
                started = true;
            }
            else {
                line.lineTo(point);
            }
        }
        painter.drawPath(line);

        painter.setPen(QColor("#263746"));
        painter.drawText(
            QRectF(plotArea.left(), height() - 26.0, plotArea.width(), 20.0),
            Qt::AlignCenter,
            "Time (seconds)"
        );
        painter.save();
        painter.translate(17.0, plotArea.center().y());
        painter.rotate(-90.0);
        painter.drawText(
            QRectF(-plotArea.height() / 2.0, -10.0,
                   plotArea.height(), 20.0),
            Qt::AlignCenter,
            "Value"
        );
        painter.restore();
    }

private:
    const datatable::DataTable& table_;
};

class LiveViewer final : public QWidget
{
public:
    explicit LiveViewer(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        table_.addColumn("Time (s)", datatable::CellType::floating_point);
        table_.addColumn("Value", datatable::CellType::floating_point);

        auto* layout = new QVBoxLayout(this);
        tableWidget_ = new QTableWidget(this);
        tableWidget_->setColumnCount(2);
        tableWidget_->setHorizontalHeaderLabels({"Time (s)", "Value"});
        tableWidget_->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch
        );
        tableWidget_->verticalHeader()->setVisible(false);
        tableWidget_->setEditTriggers(QTableWidget::NoEditTriggers);
        tableWidget_->setSelectionMode(QTableWidget::NoSelection);

        auto* plot = new LivePlot(table_, this);
        auto* plotLayout = new QVBoxLayout();
        plotLayout->addWidget(plot);
        auto* splitLayout = new QHBoxLayout();
        splitLayout->addWidget(tableWidget_, 2);
        splitLayout->addLayout(plotLayout, 3);
        layout->addLayout(splitLayout);

        auto* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this, plot]() {
            appendSample();
            plot->update();
        });
        timer->start(timerIntervalMs);

        setWindowTitle("DataTable Native Live Viewer");
        resize(1050, 560);
    }

private:
    void appendSample()
    {
        const double time = sampleIndex_ * sampleIntervalSeconds;
        const double value = std::sin(time) * 10.0;

        if (table_.rowCount() == maximumSamples) {
            table_.removeRow(0);
            tableWidget_->removeRow(0);
        }

        table_.addRow(datatable::Row{time, value});

        const int row = tableWidget_->rowCount();
        tableWidget_->insertRow(row);
        tableWidget_->setItem(
            row,
            0,
            new QTableWidgetItem(QString::number(time, 'f', 1))
        );
        tableWidget_->setItem(
            row,
            1,
            new QTableWidgetItem(QString::number(value, 'f', 2))
        );
        tableWidget_->scrollToBottom();
        ++sampleIndex_;
    }

    datatable::DataTable table_;
    QTableWidget* tableWidget_ = nullptr;
    std::size_t sampleIndex_ = 0;
};

NativeVisual makeVisual(const std::string& mode,
                        const std::string& csvPath)
{
    const datatable::DataTable table = loadCsvTable(csvPath);
    if (mode == "bike-day-top") {
        return makeBikeVisual(table, false, false);
    }
    if (mode == "bike-day-bottom") {
        return makeBikeVisual(table, false, true);
    }
    if (mode == "bike-hour-top") {
        return makeBikeVisual(table, true, false);
    }
    if (mode == "bike-hour-bottom") {
        return makeBikeVisual(table, true, true);
    }
    if (mode == "solar-top") {
        return makeSolarVisual(table, false);
    }
    if (mode == "solar-bottom") {
        return makeSolarVisual(table, true);
    }
    if (mode == "correlation") {
        return makeCorrelationVisual(table);
    }
    if (mode == "summary") {
        return makeSummaryVisual(table);
    }
    if (mode == "formatted") {
        return makeFormattedVisual(table, false);
    }
    if (mode == "formatted-preview") {
        return makeFormattedVisual(table, true);
    }
    throw std::invalid_argument("Unknown visual mode: " + mode);
}

void printUsage()
{
    std::cerr
        << "Usage:\n"
        << "  qt_live_viewer [input.csv]\n"
        << "  qt_live_viewer --show <mode> <input.csv>\n"
        << "  qt_live_viewer --render <mode> <input.csv> <output.png>\n"
        << "Modes: bike-day-top, bike-day-bottom, bike-hour-top, "
           "bike-hour-bottom, solar-top, solar-bottom, correlation, "
           "summary, formatted, formatted-preview\n";
}

int renderVisual(QApplication& application,
                 const std::string& mode,
                 const std::string& inputPath,
                 const std::string& outputPath)
{
    NativeVisual visual = makeVisual(mode, inputPath);
    NativeVisualViewer viewer(visual);
    viewer.ensurePolished();
    if (viewer.layout()) {
        viewer.layout()->activate();
    }
    application.processEvents();

    const QFileInfo outputInfo(QString::fromStdString(outputPath));
    if (!QDir().mkpath(outputInfo.absolutePath())) {
        throw std::runtime_error(
            "Could not create output directory: " + outputInfo.absolutePath().toStdString()
        );
    }
    QPixmap image(viewer.size());
    image.fill(Qt::transparent);
    QPainter painter(&image);
    viewer.render(&painter);
    painter.end();
    if (image.isNull() ||
        !image.save(QString::fromStdString(outputPath), "PNG")) {
        throw std::runtime_error("Could not save PNG image: " + outputPath);
    }
    std::cout << "Created " << outputPath << " ("
              << image.width() << "x" << image.height() << ")\n";
    return 0;
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    try {
        if (argc >= 2 && std::string(argv[1]) == "--render") {
            if (argc != 5) {
                printUsage();
                return 2;
            }
            return renderVisual(application, argv[2], argv[3], argv[4]);
        }
        if (argc >= 2 && std::string(argv[1]) == "--show") {
            if (argc != 4) {
                printUsage();
                return 2;
            }
            NativeVisual visual = makeVisual(argv[2], argv[3]);
            NativeVisualViewer viewer(visual);
            viewer.show();
            return application.exec();
        }
        if (argc > 2) {
            printUsage();
            return 2;
        }
        if (argc > 1) {
            DatasetViewer viewer(argv[1]);
            viewer.show();
            return application.exec();
        }

        LiveViewer viewer;
        viewer.show();
        return application.exec();
    }
    catch (const std::exception& error) {
        std::cerr << "Could not start the DataTable Qt viewer: "
                  << error.what() << '\n';
        return 1;
    }
}
