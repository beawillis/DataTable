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

} // namespace datatable
