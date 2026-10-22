#include "datatable/export.hpp"

#include "datatable/table.hpp"

#include <ostream>
#include <stdexcept>
#include <string>

namespace datatable {
namespace {

void ensure_writable(const std::ostream& output)
{
	// Fail before writing when the caller supplied an unusable stream.
	if (!output) {
		throw std::ios_base::failure("Export output stream is not writable.");
	}
}

std::string csv_field(const std::string& value)
{
	// CSV fields containing separators or line breaks must be quoted.
	if (value.find_first_of(",\"\r\n") == std::string::npos) {
		return value;
	}

	std::string escaped;
	escaped.reserve(value.size() + 2);
	escaped.push_back('\"');
	for (const char character : value) {
		if (character == '\"') {
			escaped.push_back('\"');
		}
		escaped.push_back(character);
	}
	escaped.push_back('\"');
	return escaped;
}

void write_html_escaped(const std::string& value, std::ostream& output)
{
	// Escape markup-sensitive characters so exported text cannot become markup.
	for (const char character : value) {
		switch (character) {
		case '&': output << "&amp;"; break;
		case '<': output << "&lt;"; break;
		case '>': output << "&gt;"; break;
		case '\"': output << "&quot;"; break;
		case '\'': output << "&#39;"; break;
		default: output.put(character); break;
		}
	}
}

} // namespace

void write_csv(const DataTable& table, std::ostream& output)
{
	ensure_writable(output);

	const auto& names = table.columnNames();
	if (!names.empty()) {
		// Emit the escaped header row before the table records.
		for (std::size_t index = 0; index < names.size(); ++index) {
			if (index != 0) {
				output.put(',');
			}
			output << csv_field(names[index]);
		}
		output << "\r\n";
	}

	for (const Row& row : table.rows()) {
		for (std::size_t index = 0; index < row.size(); ++index) {
			if (index != 0) {
				output.put(',');
			}
			output << csv_field(row[index].toString());
		}
		output << "\r\n";
	}

	ensure_writable(output);
}

void write_html(const DataTable& table, std::ostream& output)
{
	ensure_writable(output);

	// Build a complete HTML document containing the current column headers.
	output << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
			  "<meta charset=\"UTF-8\">\n<title>DataTable Export</title>\n"
			  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
			  "<style>\n"
			  "body{font-family:Segoe UI,Arial,sans-serif;margin:2rem;color:#1f2937;"
			  "background:#f3f6fa}\n"
			  "table{border-collapse:collapse;background:#fff;min-width:60%;"
			  "box-shadow:0 2px 10px #00000014}\n"
			  "th,td{padding:.65rem .9rem;border:1px solid #d8e0ea;text-align:left}\n"
			  "th{background:#174a7e;color:#fff}\n"
			  "tbody tr:nth-child(even){background:#edf3f8}\n"
			  "tbody tr:hover{background:#dceafa}\n"
			  "</style>\n</head>\n<body>\n<table>\n<thead>\n<tr>";
	for (const auto& name : table.columnNames()) {
		output << "<th>";
		write_html_escaped(name, output);
		output << "</th>";
	}
	output << "</tr>\n</thead>\n<tbody>\n";

	// Export each complete row so HTML preserves the same records as CSV.
	for (const Row& row : table.rows()) {
		output << "<tr>";
		for (const Cell& cell : row.cells()) {
			output << "<td>";
			write_html_escaped(cell.toString(), output);
			output << "</td>";
		}
		output << "</tr>\n";
	}

	output << "</tbody>\n</table>\n</body>\n</html>\n";

	ensure_writable(output);
}

} // namespace datatable
