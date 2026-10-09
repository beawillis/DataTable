#ifndef DATATABLE_CSV_HPP
#define DATATABLE_CSV_HPP

#include <iosfwd>
#include <string>
#include <vector>

namespace datatable {

using CsvRecord = std::vector<std::string>;
using CsvData = std::vector<CsvRecord>;

// Parse CSV into strings; this does not infer types or create a DataTable.
CsvData parseCsv(std::istream& input);
CsvData parseCsv(const std::string& input);

} // namespace datatable

#endif // DATATABLE_CSV_HPP
