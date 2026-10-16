#ifndef DATATABLE_EXPORT_HPP
#define DATATABLE_EXPORT_HPP

#include <iosfwd>

namespace datatable {

class DataTable;

// Write the table as CSV or HTML to an existing output stream.
// CSV includes headers and all row values; stream failures raise
// std::ios_base::failure. HTML currently exports the schema.
void write_csv(const DataTable& table, std::ostream& output);
void write_html(const DataTable& table, std::ostream& output);

} // namespace datatable

#endif // DATATABLE_EXPORT_HPP
