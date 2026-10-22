# Dataset files

- `input/day.csv` and `input/hour.csv` are the UCI Bike Sharing dataset CSVs.
- Use `day.csv` for smaller interactive tests and `hour.csv` for larger tests.
- Put generated exports and other test results in `output/`; its contents are
  ignored by Git so test runs do not add generated files to the repository.
- From the repository root, build the real-dataset tool:

  ```powershell
  cmake --build build --config Debug --target dataset_example
  ```

  Then choose an operation:

  ```powershell
  # Sort by cnt, highest first; column defaults to cnt.
  .\build\examples\Debug\dataset_example.exe sort data\input\day.csv data\output\day-sorted.csv

  # Keep only spring records (season == 1).
  .\build\examples\Debug\dataset_example.exe filter data\input\day.csv data\output\spring.csv season 1

  # Export the original dataset as CSV or a complete HTML document.
  .\build\examples\Debug\dataset_example.exe export data\input\day.csv data\output\day-copy.csv
  .\build\examples\Debug\dataset_example.exe export data\input\day.csv data\output\day.html

  # Format a readable preview in the console (20 rows by default).
  .\build\examples\Debug\dataset_example.exe format data\input\day.csv 10

  # Group and count records by season.
  .\build\examples\Debug\dataset_example.exe group data\input\hour.csv season
  ```

  Replace `day.csv` with `hour.csv` to use the larger hourly dataset. Column
  names are taken from the CSV header. Filter values must match the column's
  type (for example, `season 1` or `dteday 2011-01-01`). Use `Release` instead
  of `Debug` in the executable path if you build the Release configuration.

The tool normalizes integer-looking values to decimal notation in numeric
columns that also contain decimal values. This compatibility step happens in
memory; it does not change the downloaded source CSVs. The core CSV loader
continues to reject mixed non-null cell types.

The CSV files were downloaded from the
[UCI Bike Sharing dataset](https://archive.ics.uci.edu/dataset/275/bike+sharing+dataset).
The library reads local CSV streams; it does not download URLs or open ZIP
archives itself.
