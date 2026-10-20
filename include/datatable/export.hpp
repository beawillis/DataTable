#ifndef DATATABLE_EXPORT_HPP
#define DATATABLE_EXPORT_HPP

#include <iosfwd>

namespace datatable {

class DataTable;

// Write the complete table as CSV or HTML to an existing output stream.
// Stream failures raise std::ios_base::failure.
void write_csv(const DataTable& table, std::ostream& output);
void write_html(const DataTable& table, std::ostream& output);

} // namespace datatable

#endif // DATATABLE_EXPORT_HPP
