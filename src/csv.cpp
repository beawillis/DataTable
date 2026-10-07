#include "datatable/csv.hpp"

#include <sstream>
#include <stdexcept>

namespace datatable {

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

	if (input.bad()) {
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

} // namespace datatable
