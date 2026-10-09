#ifndef DATATABLE_EXPORT_HPP
#define DATATABLE_EXPORT_HPP

#include <iosfwd>

namespace datatable {

class DataTable;

// Write a CSV header or complete HTML document to an existing stream.
// Until DataTable exposes row values, both functions reject nonempty tables
// with std::logic_error. Stream failures raise std::ios_base::failure.
void write_csv(const DataTable& table, std::ostream& output);
void write_html(const DataTable& table, std::ostream& output);

} // namespace datatable

#endif // DATATABLE_EXPORT_HPP
