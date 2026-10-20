#include <cassert>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <streambuf>

#include "datatable/export.hpp"
#include "datatable/table.hpp"

namespace {

class FailingBuffer : public std::streambuf
{
protected:
	// Always report a write failure so exporters can be tested with a bad stream.
	int_type overflow(int_type) override
	{
		return traits_type::eof();
	}
};

void test_csv_header_and_escaping()
{
	// Headers containing CSV-special characters must be quoted and escaped.
	datatable::DataTable table;
	table.addColumn("Name");
	table.addColumn("value, with comma");
	table.addColumn("quote\" and newline\n");
	table.addColumn("carriage\rreturn");

	std::ostringstream output;
	datatable::write_csv(table, output);

	// The exporter writes one escaped header record with CRLF termination.
	assert(output.str() == "Name,\"value, with comma\",\"quote\"\" and newline\n\",\"carriage\rreturn\"\r\n");
}

void test_csv_exports_rows_and_cell_values()
{
	// Build a table with text, an escaped quote, a number, and a null field.
	const datatable::DataTable table = datatable::DataTable::fromCsv(
		"Name,Note,Score\n"
		"Ada,\"ready, now\",98.5\n"
		"Grace,\"say \"\"hello\"\"\","
		"\n"
	);

	std::ostringstream output;
	datatable::write_csv(table, output);

	// Headers and complete rows must be exported in their original order.
	assert(output.str() ==
		"Name,Note,Score\r\n"
		"Ada,\"ready, now\",98.5\r\n"
		"Grace,\"say \"\"hello\"\"\",\r\n");
}

void test_html_document_and_escaping()
{
	// Headers and cell values must be escaped as text, not markup.
	const datatable::DataTable table = datatable::DataTable::fromCsv(
		"\"<Name & \"\"title\"\" 'label'>\",plain\n"
		"\"<Ada & Grace>\",\"ready > now\"\n"
	);

	std::ostringstream output;
	datatable::write_html(table, output);

	const std::string html = output.str();
	// Verify the document structure and confirm no executable markup is added.
	assert(html.find("<!DOCTYPE html>") == 0);
	assert(html.find("<th>&lt;Name &amp; &quot;title&quot; &#39;label&#39;&gt;</th>") != std::string::npos);
	assert(html.find("<th>plain</th>") != std::string::npos);
	assert(html.find("<tbody>\n<tr><td>&lt;Ada &amp; Grace&gt;</td><td>ready &gt; now</td></tr>\n</tbody>") != std::string::npos);
	assert(html.find("<script") == std::string::npos);
}

void test_tables_without_columns()
{
	// A table without a schema produces empty CSV but valid HTML structure.
	datatable::DataTable table;

	std::ostringstream csv;
	datatable::write_csv(table, csv);
	assert(csv.str().empty());

	std::ostringstream html;
	datatable::write_html(table, html);
	assert(html.str().find("<table>") != std::string::npos);
	assert(html.str().find("<thead>\n<tr></tr>") != std::string::npos);
	assert(html.str().find("<tbody>\n</tbody>") != std::string::npos);
}

void test_failed_output_stream_is_reported()
{
	// Exporters must report output failures instead of silently succeeding.
	datatable::DataTable table;
	table.addColumn("Name");
	FailingBuffer buffer;
	std::ostream output(&buffer);

	bool exception_thrown = false;
	try {
		datatable::write_csv(table, output);
	} catch (const std::ios_base::failure&) {
		exception_thrown = true;
	}
	assert(exception_thrown);

	FailingBuffer html_buffer;
	std::ostream html_output(&html_buffer);
	exception_thrown = false;
	try {
		datatable::write_html(table, html_output);
	} catch (const std::ios_base::failure&) {
		exception_thrown = true;
	}
	assert(exception_thrown);
}

} // namespace

int main()
{
	test_csv_header_and_escaping();
	test_csv_exports_rows_and_cell_values();
	test_html_document_and_escaping();
	test_tables_without_columns();
	test_failed_output_stream_is_reported();

	std::cout << "All export tests passed.\n";
	return 0;
}
