#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "datatable/csv.hpp"

using datatable::CsvData;
using datatable::parseCsv;

void test_plain_records_and_empty_fields()
{
	// Empty fields and blank lines are preserved as data.
	const CsvData records = parseCsv("name,age,city\r\nAda,37,\r\n\n");

	assert(records.size() == 3);
	assert((records[0] == datatable::CsvRecord{"name", "age", "city"}));
	assert((records[1] == datatable::CsvRecord{"Ada", "37", ""}));
	assert((records[2] == datatable::CsvRecord{""}));
}

void test_quoted_fields_and_embedded_newlines()
{
	const CsvData records = parseCsv("\"last, first\",note\r\n\"Lovelace, Ada\",\"line 1\nline 2\"\n");

	assert(records.size() == 2);
	assert((records[0] == datatable::CsvRecord{"last, first", "note"}));
	assert((records[1] == datatable::CsvRecord{"Lovelace, Ada", "line 1\nline 2"}));
}

void test_escaped_quotes_and_trailing_delimiter()
{
	const CsvData records = parseCsv("\"say \"\"hello\"\"\",end,");

	assert(records.size() == 1);
	assert((records[0] == datatable::CsvRecord{"say \"hello\"", "end", ""}));
}

void test_stream_input_and_empty_input()
{
	std::istringstream stream("one,two\n");

	assert((parseCsv(stream) == CsvData{datatable::CsvRecord{"one", "two"}}));
	assert(parseCsv("").empty());
}

void test_malformed_quotes_are_rejected()
{
	// Reject an unclosed quote, a quote inside plain text, and text after a closing quote.
	for (const std::string input : {"\"unfinished", "un\"expected", "\"closed\"x"}) {
		bool exception_thrown = false;
		try {
			parseCsv(input);
		} catch (const std::invalid_argument&) {
			exception_thrown = true;
		}
		assert(exception_thrown);
	}
}

int main()
{
	test_plain_records_and_empty_fields();
	test_quoted_fields_and_embedded_newlines();
	test_escaped_quotes_and_trailing_delimiter();
	test_stream_input_and_empty_input();
	test_malformed_quotes_are_rejected();

	std::cout << "All CSV parser tests passed.\n";
	return 0;
}
