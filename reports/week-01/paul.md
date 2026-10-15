# DataTable — Week 1 Progress Report

**Contributor:** paul

### Paul — Formatting and Styling

- **Completed work:** Implemented the Week 1 formatting foundation for a
  standalone `Column`. `formatColumn()` renders a named column with optional
  headers, padding, borders, and left, center, or right alignment. `Style`
  stores presentation choices separately so formatting does not modify the
  source values.

- **Files changed:** [`formatter.hpp`](../../include/datatable/formatter.hpp),
  [`formatter.cpp`](../../src/formatter.cpp), [`style.hpp`](../../include/datatable/style.hpp),
  [`style.cpp`](../../src/style.cpp), [`test_formatter.cpp`](../../tests/test_formatter.cpp),
  [`test_style.cpp`](../../tests/test_style.cpp), and
  [`tests/CMakeLists.txt`](../../tests/CMakeLists.txt).

- **Tests and validation:** Added tests for default headers, borders, padding,
  alignment settings, compact output, and preservation of the source column.
  The formatter and style tests passed after a successful build.

- **Open decisions or blockers:** The current Week 1 formatter presents one
  standalone `Column` because `DataTable` does not yet expose integrated rows
  and columns. Full multi-column formatting, numeric precision options, and
  table-level styling should be added after the shared table interface is
  extended.

#### Class structure and implementation

Paul's formatting work introduces the `Style` class and uses the existing
`Column` and `Cell` classes as its input.

- **Class created:** `Style` represents presentation settings without owning
  table data. It stores private `alignment_`, `show_header_`, `show_borders_`,
  and `padding_` members.
- **Member functions:** Public accessors `alignment()`, `showHeader()`,
  `showBorders()`, and `padding()` read the settings. Public setters update
  those settings. The free function `formatColumn()` reads a `Column` and a
  `Style` and returns formatted text.
- **Supporting classes:** `Column` supplies the public `name()`, `cells()`,
  and `size()` interface. Each `Cell` supplies `toString()` so formatting can
  display its value without accessing private storage.
- **Public and private design:** Style settings are public through controlled
  getter and setter functions because callers need to customize presentation.
  The settings themselves remain private so formatting code cannot bypass the
  class interface. `Column` and `Cell` also keep their stored data private,
  which prevents presentation from changing the underlying values.
- **How it is used:** A caller creates a `Style`, optionally changes its
  alignment, header, border, or padding settings, then passes it to
  `formatColumn()`. The formatter calculates the widest displayed value,
  aligns every row to that width, and returns a new string. The input column
  remains unchanged.

The design answers the implementation questions directly: `Style` was created
to group presentation choices; it holds alignment and display flags; its
accessors and setters operate on those settings; and private members preserve
the style object's state while public functions make it configurable by the
rest of the program.




