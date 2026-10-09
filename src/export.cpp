#include "datatable/export.hpp"

#include "datatable/table.hpp"

#include <ostream>
#include <stdexcept>
#include <string>

namespace datatable {
namespace {

void ensure_writable(const std::ostream& output)
{
	if (!output) {
		throw std::ios_base::failure("Export output stream is not writable.");
	}
}

void ensure_schema_only(const DataTable& table)
{
	if (table.rowCount() != 0) {
		throw std::logic_error(
			"DataTable row values are not exposed to the export module yet."
		);
	}
}

std::string csv_field(const std::string& value)
{
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
	ensure_schema_only(table);

	const auto& names = table.columnNames();
	if (!names.empty()) {
		for (std::size_t index = 0; index < names.size(); ++index) {
			if (index != 0) {
				output.put(',');
			}
			output << csv_field(names[index]);
		}
		output << "\r\n";
	}

	ensure_writable(output);
}

void write_html(const DataTable& table, std::ostream& output)
{
	ensure_writable(output);
	ensure_schema_only(table);

	output << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
			  "<meta charset=\"UTF-8\">\n<title>DataTable Export</title>\n"
			  "</head>\n<body>\n<table>\n<thead>\n<tr>";
	for (const auto& name : table.columnNames()) {
		output << "<th>";
		write_html_escaped(name, output);
		output << "</th>";
	}
	output << "</tr>\n</thead>\n<tbody>\n</tbody>\n</table>\n"
			  "</body>\n</html>\n";

	ensure_writable(output);
}

} // namespace datatable
