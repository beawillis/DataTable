# Dataset files

[Project overview](../README.md) · [Build and test](../README.md#build-and-test) ·
[API overview](../README.md#api-overview) · [Reports](../reports/README.md)

## Contents

- [Open a CSV in the native Qt viewer](#open-a-csv-in-the-native-qt-viewer)
  - [Configure and build](#configure-and-build)
  - [View the included datasets](#view-the-included-datasets)
- [Recreate the project visuals with Qt](#recreate-the-project-visuals-with-qt)
- [Transform datasets with the command-line example](#transform-datasets-with-the-command-line-example)
- [Dataset notes](#dataset-notes)

- `input/day.csv` and `input/hour.csv` are the UCI Bike Sharing datasets.
  `day.csv` is smaller; `hour.csv` contains the larger hourly dataset.
- `input/sza.csv` contains solar zenith angle measurements from the `gt`
  package's [`sza` data](https://raw.githubusercontent.com/rstudio/gt/master/data-raw/sza.csv).
- `input/gtcars.csv` is the 47-car dataset from the `gt` package. Use
  `input/gtcars-numeric.csv` to view its seven numeric specification fields.
- The `sza` and `gtcars` data are adapted from the
  [`gt` package](https://github.com/rstudio/gt), which is licensed under MIT.
  Keep the source links and attribution when redistributing them.
- Put generated CSV results in `output/`; this directory is ignored by Git.

## Open a CSV in the native Qt viewer

The optional Qt application displays CSV data in a native desktop table.
Numeric cells are color-coded from lower to higher values within each column;
text and missing values remain uncolored. The viewer reads CSV fields as text
for display, so it can also show datasets whose numeric columns contain a mix
of integer-looking and decimal values.

### Configure and build

Install Qt 5 or Qt 6 with the **Widgets** development component, plus CMake and
a compiler compatible with that Qt kit. A MinGW Qt kit must use its matching
MinGW compiler. From the repository root, replace the example Qt version and
paths below with your installation:

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:PATH"

cmake -S . -B build-qt -G Ninja `
  -DCMAKE_MAKE_PROGRAM="C:\Qt\Tools\Ninja\ninja.exe" `
  -DCMAKE_CXX_COMPILER="C:\Qt\Tools\mingw1310_64\bin\g++.exe" `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.12.0\mingw_64" `
  -DDATATABLE_BUILD_QT_LIVE_VIEWER=ON `
  -DDATATABLE_BUILD_EXAMPLES=ON `
  -DDATATABLE_BUILD_TESTS=OFF

cmake --build build-qt --target qt_live_viewer
```

### View the included datasets

Run one command at a time from the repository root:

```powershell
$env:PATH = "C:\Qt\6.12.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;$env:PATH"

.\build-qt\examples\qt_live_viewer.exe .\data\input\day.csv
.\build-qt\examples\qt_live_viewer.exe .\data\input\hour.csv
.\build-qt\examples\qt_live_viewer.exe .\data\input\sza.csv
.\build-qt\examples\qt_live_viewer.exe .\data\input\gtcars-numeric.csv
```

The same viewer can display any CSV with a header row and consistent field
counts. For a regular CSV, numeric cells are colored against the minimum and
maximum of their own column; columns use a fixed initial width so the full
dataset does not need to be scanned just to size the headers.

Close the window before launching another dataset. To see the live
table-and-graph demonstration instead, run the viewer without a file argument:

```powershell
.\build-qt\examples\qt_live_viewer.exe
```

## Recreate the project visuals with Qt

The Qt application has native modes for the same project visuals: daily Bike
Sharing, hourly Bike Sharing, solar zenith, car correlations, the sorted
seasonal summary, and two formatted daily-table previews. To view a visual in a
desktop window, use `--show <mode> <csv>`:

```powershell
.\build-qt\examples\qt_live_viewer.exe --show bike-day-top .\data\input\day.csv
.\build-qt\examples\qt_live_viewer.exe --show bike-hour-top .\data\input\hour.csv
.\build-qt\examples\qt_live_viewer.exe --show solar-top .\data\input\sza.csv
.\build-qt\examples\qt_live_viewer.exe --show correlation .\data\input\gtcars-numeric.csv
.\build-qt\examples\qt_live_viewer.exe --show summary .\data\input\day.csv
```

Use `--render <mode> <csv> <output.png>` to generate an image directly from
the native Qt widgets. Run these commands from the repository root to recreate
all the visuals in `reports/visuals/qt-native/`:

```powershell
$viewer = ".\build-qt\examples\qt_live_viewer.exe"
$output = ".\reports\visuals\qt-native"

& $viewer --render bike-day-top .\data\input\day.csv "$output\bike-day-heatmap-top.png"
& $viewer --render bike-day-bottom .\data\input\day.csv "$output\bike-day-heatmap-bottom.png"
& $viewer --render bike-hour-top .\data\input\hour.csv "$output\bike-hour-heatmap-top.png"
& $viewer --render bike-hour-bottom .\data\input\hour.csv "$output\bike-hour-heatmap-bottom.png"
& $viewer --render solar-top .\data\input\sza.csv "$output\solar-zenith-heatmap-top.png"
& $viewer --render solar-bottom .\data\input\sza.csv "$output\solar-zenith-heatmap-bottom.png"
& $viewer --render correlation .\data\input\gtcars-numeric.csv "$output\gtcars-correlation-heatmap.png"
& $viewer --render summary .\data\input\day.csv "$output\sorted-season.png"
& $viewer --render formatted .\data\input\day.csv "$output\formatted.png"
& $viewer --render formatted-preview .\data\input\day.csv "$output\formatted-preview.png"
```

The daily and hourly heatmaps group rentals (`cnt`) by month and year, or by
hour and season, then color the averages. The solar visual reshapes the angle
measurements for latitude 20 into a month-by-time heatmap and splits the months
into two images, keeping all 17 time columns visible in each. The correlation
visual computes Pearson correlations for the seven numeric car fields using a
blue-white-red scale. The summary filters daily records to 2012, groups by
season, calculates rental statistics, and sorts by total rentals. The formatted
views show rows 1–10 and 11–20 of `day.csv`.

The PNG captures are generated directly from native Qt widgets. The commands
create the `qt-native` directory if it does not already exist.

## Transform datasets with the command-line example

The non-Qt `dataset_example` can sort, filter, format, group, summarize, and
export CSV data. It writes CSV output; use `qt_live_viewer` to inspect a CSV
result with the native Qt interface.

Build the command-line example in the existing standard build:

```powershell
cmake --build build --config Debug --target dataset_example
```

Examples:

```powershell
# Sort by rental count, highest first.
.\build\examples\Debug\dataset_example.exe sort .\data\input\day.csv .\data\output\day-sorted.csv

# Keep only spring records (season == 1).
.\build\examples\Debug\dataset_example.exe filter .\data\input\day.csv .\data\output\spring.csv season 1

# Format the first ten rows in the console.
.\build\examples\Debug\dataset_example.exe format .\data\input\day.csv 10

# Group hourly records by season.
.\build\examples\Debug\dataset_example.exe group .\data\input\hour.csv season

# Summarize 2012 daily rentals by season.
.\build\examples\Debug\dataset_example.exe summary .\data\input\day.csv .\data\output\bike-season-summary.csv

# Summarize solar zenith angles by month for latitude 20.
.\build\examples\Debug\dataset_example.exe sza-summary .\data\input\sza.csv .\data\output\solar-zenith-summary.csv 20
```

Display a generated result in the native viewer, for example:

```powershell
.\build-qt\examples\qt_live_viewer.exe .\data\output\bike-season-summary.csv
```

That command displays the seasonal summary as a color-scaled Qt table. It
doesn't draw a seasonal chart or compute a two-dimensional heatmap. To see the
all-row data with numeric cells colored, open `day.csv` or `hour.csv` directly.

Replace `Debug` with `Release` in the command-line executable path when using
a Release build. The Qt kit, compiler, and CMake generator must be compatible;
use a separate build directory if you switch toolchains.

## Dataset notes

- The Bike Sharing files come from the
  [UCI Bike Sharing dataset](https://archive.ics.uci.edu/dataset/275/bike+sharing+dataset).
- `sza.csv` has 816 rows with latitude, month, true solar time, and solar
  zenith angle. Empty fields represent source `NA` values.
- `gtcars-numeric.csv` is a seven-column projection of the car dataset:
  `hp`, `hp_rpm`, `trq`, `trq_rpm`, `mpg_c`, `mpg_h`, and `msrp`.
-   The Qt viewer supports both per-column color-coded CSV tables and the
  pre-aggregated native heatmap and summary modes documented above.
