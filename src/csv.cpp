#include "datatable/csv.hpp"

#include "datatable/table.hpp"

#include <cerrno>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace datatable {
namespace {

Cell cellFromCsvField(const std::string& field)
{
	if (field.empty()) {
		return Cell(nullptr);
	}

	// Preserve whitespace because it is part of the CSV field value.
	if (std::isspace(static_cast<unsigned char>(field.front())) ||
		std::isspace(static_cast<unsigned char>(field.back()))) {
		return Cell(field);
	}

	if (field == "true") {
		return Cell(true);
	}

	if (field == "false") {
		return Cell(false);
	}

	// Keep integer cells exact; use double only for non-integer numeric fields.
	char* integerEnd = nullptr;
	errno = 0;
	const long long integer = std::strtoll(field.c_str(), &integerEnd, 10);
	if (integerEnd != field.c_str() && *integerEnd == '\0') {
		if (errno != ERANGE) {
			return Cell(static_cast<std::int64_t>(integer));
		}

		// Keep unsigned integer precision when the value exceeds INT64_MAX.
		if (field.front() != '-') {
			char* unsignedEnd = nullptr;
			errno = 0;
			const unsigned long long unsignedInteger =
				std::strtoull(field.c_str(), &unsignedEnd, 10);
			if (unsignedEnd != field.c_str() && *unsignedEnd == '\0' &&
				errno != ERANGE) {
				return Cell(static_cast<std::uint64_t>(unsignedInteger));
			}
		}

		return Cell(field);
	}

	char* numberEnd = nullptr;
	errno = 0;
	const double number = std::strtod(field.c_str(), &numberEnd);
	if (numberEnd != field.c_str() && *numberEnd == '\0' &&
		errno != ERANGE) {
		return Cell(number);
	}

	return Cell(field);
}

} // namespace

CsvData parseCsv(std::istream& input)
{
	CsvData records;
	CsvRecord record;
	std::string field;
	// Track quoted fields and whether their closing quote has been consumed.
	bool in_quotes = false;
	bool after_quote = false;
	bool record_started = false;
	char character = '\0';

	// Save the final field too, including an empty one after a trailing comma.
	const auto finish_record = [&]() {
		record.push_back(field);
		records.push_back(record);
		record.clear();
		field.clear();
		record_started = false;
	};

	while (input.get(character)) {
		if (in_quotes) {
			if (character == '"') {
				// Two quotes inside a quoted field represent one literal quote.
				if (input.peek() == '"') {
					input.get(character);
					field.push_back('"');
				} else {
					// A closing quote may only be followed by a delimiter or line end.
					in_quotes = false;
					after_quote = true;
				}
			} else {
				field.push_back(character);
			}
			continue;
		}

		if (after_quote && character != ',' && character != '\r' && character != '\n') {
			throw std::invalid_argument("Unexpected character after quoted CSV field.");
		}

		if (character == '"') {
			if (!field.empty() || after_quote) {
				throw std::invalid_argument("Unexpected quote in unquoted CSV field.");
			}
			in_quotes = true;
			record_started = true;
		} else if (character == ',') {
			record.push_back(field);
			field.clear();
			after_quote = false;
			record_started = true;
		} else if (character == '\r' || character == '\n') {
			// Treat CRLF as one record separator while also accepting lone CR or LF.
			if (character == '\r' && input.peek() == '\n') {
				input.get(character);
			}
			finish_record();
			after_quote = false;
		} else {
			if (after_quote) {
				throw std::invalid_argument("Unexpected character after quoted CSV field.");
			}
			field.push_back(character);
			record_started = true;
		}
	}

	// EOF is a normal termination condition; other stream failures indicate
	// that the CSV source could not be read completely.
	if (input.bad() || (input.fail() && !input.eof())) {
		throw std::ios_base::failure("Failed while reading CSV input.");
	}
	if (in_quotes) {
		throw std::invalid_argument("Unterminated quoted CSV field.");
	}
	// Do not add a phantom record when the input ends immediately after a newline.
	if (record_started || !record.empty() || !field.empty() || after_quote) {
		finish_record();
	}

	return records;
}

CsvData parseCsv(const std::string& input)
{
	std::istringstream stream(input);
	return parseCsv(stream);
}

DataTable DataTable::fromCsv(std::istream& input)
{
	const CsvData records = parseCsv(input);
	DataTable table;

	if (records.empty()) {
		return table;
	}

	// The first parsed record is the schema; later records become table rows.
	for (const std::string& header : records.front()) {
		table.addColumn(header);
	}

	for (std::size_t recordIndex = 1;
		 recordIndex < records.size();
		 ++recordIndex) {
		const CsvRecord& record = records[recordIndex];
		if (record.size() != records.front().size()) {
			// Reject ragged input instead of building a misaligned table.
			throw std::invalid_argument(
				"CSV record width does not match the header width."
			);
		}

		std::vector<Cell> cells;
		cells.reserve(record.size());
		for (const std::string& field : record) {
			cells.emplace_back(cellFromCsvField(field));
		}
		table.addRow(Row(std::move(cells)));
	}

	return table;
}

DataTable DataTable::fromCsv(const std::string& input)
{
	std::istringstream stream(input);
	return fromCsv(stream);
}

} // namespace datatable
