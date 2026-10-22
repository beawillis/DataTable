#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

#include "datatable/csv.hpp"
#include "datatable/table.hpp"

using datatable::CsvData;
using datatable::DataTable;
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

void test_csv_to_table_loading()
{
	// Verify that CSV fields become typed cells in the shared DataTable.
	const DataTable table = DataTable::fromCsv(
		"Name,Age,Active,Score,Note\n"
		"Ada,36,true,98.5,\n"
		"Grace,28,false,91.25,ready\n"
	);

	assert(table.columnCount() == 5);
	assert(table.rowCount() == 2);
	assert(table.column("Name")[0].get<std::string>() == "Ada");
	assert(table.column("Age")[1].get<std::int64_t>() == 28);
	assert(table.column("Active")[1].get<bool>() == false);
	assert(table.column("Score")[0].get<double>() == 98.5);
	assert(table.column("Note")[0].isNull());
}

void test_csv_to_table_quoted_values_and_header_only()
{
	const DataTable empty = DataTable::fromCsv("");
	assert(empty.empty());
	assert(empty.columnCount() == 0);

	const DataTable header_only = DataTable::fromCsv("Name,Note\n");
	assert(header_only.columnCount() == 2);
	assert(header_only.rowCount() == 0);

	const DataTable table = DataTable::fromCsv(
		"Name,Note\n"
		"\"Lovelace, Ada\",\"line 1\nline 2\"\n"
	);
	assert(table.rowCount() == 1);
	assert(table.rows()[0][0].get<std::string>() == "Lovelace, Ada");
	assert(table.rows()[0][1].get<std::string>() == "line 1\nline 2");
}

void test_csv_to_table_numeric_boundaries_and_whitespace()
{
	// Ensure integer limits stay exact and field whitespace is not discarded.
	const std::string largest_unsigned =
		std::to_string((std::numeric_limits<std::uint64_t>::max)());
	const std::string smallest_signed =
		std::to_string((std::numeric_limits<std::int64_t>::min)());
	const std::string beyond_unsigned = largest_unsigned + "0";
	const std::string below_signed = "-" + largest_unsigned + "0";
	const DataTable table = DataTable::fromCsv(
		"unsigned,large,spaced,min_signed,below_signed\n" +
		largest_unsigned + "," + beyond_unsigned + ", 7," +
		smallest_signed + "," + below_signed + "\n"
	);

	assert(table.rows()[0][0].get<std::uint64_t>() ==
		(std::numeric_limits<std::uint64_t>::max)());
	assert(table.rows()[0][1].get<std::string>() == beyond_unsigned);
	assert(table.rows()[0][2].get<std::string>() == " 7");
	assert(table.rows()[0][3].get<std::int64_t>() ==
		(std::numeric_limits<std::int64_t>::min)());
	assert(table.rows()[0][4].get<std::string>() == below_signed);
}

void test_csv_to_table_stream_and_width_validation()
{
	std::istringstream input("Name,Age\nAda,36\n");
	const DataTable table = DataTable::fromCsv(input);
	assert(table.rowCount() == 1);
	assert(table.column("Age")[0].get<std::int64_t>() == 36);

	bool exception_thrown = false;
	try {
		(void)DataTable::fromCsv("Name,Age\nAda\n");
	} catch (const std::invalid_argument&) {
		exception_thrown = true;
	}
	assert(exception_thrown);
}

void test_missing_values_and_malformed_table_rows()
{
	// Empty fields become null cells, including leading and trailing fields.
	const DataTable table = DataTable::fromCsv(
		"Name,Age,Note\n"
		",,ready\n"
		"Ada,,\n"
	);
	assert(table.rowCount() == 2);
	assert(table.rows()[0][0].isNull());
	assert(table.rows()[0][1].isNull());
	assert(table.rows()[1][1].isNull());
	assert(table.rows()[1][2].isNull());

	// A record with too few or too many fields is rejected.
	for (const std::string input : {
			"Name,Age\nAda\n",
			"Name,Age\nAda,36,extra\n"
		}) {
		bool exception_thrown = false;
		try {
			(void)DataTable::fromCsv(input);
		}
		catch (const std::invalid_argument&) {
			exception_thrown = true;
		}
		assert(exception_thrown);
	}
}

void test_csv_stream_errors_are_rejected()
{
	// A non-EOF stream failure represents an unreadable CSV source.
	std::istringstream failed_input("Name,Age\nAda,36\n");
	failed_input.setstate(std::ios::failbit);

	bool exception_thrown = false;
	try {
		(void)parseCsv(failed_input);
	}
	catch (const std::ios_base::failure&) {
		exception_thrown = true;
	}
	assert(exception_thrown);
}

void test_large_csv_input()
{
	// Generate a repeatable large input without depending on an external file.
	std::ostringstream input;
	input << "Id,Department,Score\n";
	for (std::size_t index = 0; index < 10000; ++index) {
		input << index << ','
			  << (index % 2 == 0 ? "Engineering" : "Support") << ','
			  << (index % 1000) / 10.0 << '\n';
	}

	const DataTable table = DataTable::fromCsv(input.str());
	assert(table.columnCount() == 3);
	assert(table.rowCount() == 10000);
	assert(table.column("Id")[0].get<std::int64_t>() == 0);
	assert(table.column("Id")[9999].get<std::int64_t>() == 9999);
	assert(table.column("Department")[9998].get<std::string>() ==
		   "Engineering");
	assert(table.column("Score")[9999].get<double>() == 99.9);
}

int main()
{
	test_plain_records_and_empty_fields();
	test_quoted_fields_and_embedded_newlines();
	test_escaped_quotes_and_trailing_delimiter();
	test_stream_input_and_empty_input();
	test_malformed_quotes_are_rejected();
	test_csv_to_table_loading();
	test_csv_to_table_quoted_values_and_header_only();
	test_csv_to_table_numeric_boundaries_and_whitespace();
	test_csv_to_table_stream_and_width_validation();
	test_missing_values_and_malformed_table_rows();
	test_csv_stream_errors_are_rejected();
	test_large_csv_input();

	std::cout << "All CSV tests passed.\n";
	return 0;
}
