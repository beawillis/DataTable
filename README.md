# DataTable — High-Performance Data Table Formatting and Presentation Library

> A reusable C++ library for creating, manipulating, analyzing, formatting, and exporting tabular data.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Problem Statement](#problem-statement)
3. [Objectives](#objectives)
4. [Features](#features)
5. [Technology Stack](#technology-stack)
6. [Project Architecture](#project-architecture)
7. [Requirements](#requirements)
8. [Installation and Build](#installation-and-build)
9. [Usage](#usage)
10. [Testing](#testing)
11. [Performance and Design Considerations](#performance-and-design-considerations)
12. [Limitations](#limitations)
13. [Contributing](#contributing)
14. [Team Members](#team-members)
15. [AI Use Declaration](#ai-use-declaration)
16. [License](#license)

---

## 1. Project Overview

**Team task:** Write a concise description explaining what DataTable is, the problem it addresses, its intended users, and why a reusable C++ library is appropriate.

## 2. Problem Statement

Describe the difficulties of managing, organizing, analyzing, formatting, and exporting tabular data when reusable functionality is unavailable or fragmented.

Explain how DataTable aims to address these challenges through a modular C++ API.

## 3. Objectives

- Develop a reusable C++ library for tabular data management.
- Implement a suitable data model with typed columns.
- Support common data manipulation and analysis operations.
- Provide consistent table formatting and presentation.
- Support CSV input and CSV/HTML output.
- Apply modular software design and appropriate data structures.
- Use automated tests to verify correctness and edge cases.
- Provide build instructions, examples, and documentation.
- Use Git and GitHub for collaborative development and code review.

## 4. Features

Update this list as each feature is implemented and tested.

- [ ] Table creation and column management
- [ ] Strongly typed columns: integer, double, string, and boolean
- [ ] Row insertion, removal, and updating
- [ ] CSV file loading
- [ ] Sorting in ascending and descending order
- [ ] Data filtering
- [ ] Grouping and aggregation
- [ ] Table formatting and styling
- [ ] Headers and footers
- [ ] CSV export
- [ ] HTML export
- [ ] Input validation and error handling
- [ ] Automated unit and integration tests
- [ ] Compilable examples
- [ ] CMake-based build and installation

## 5. Technology Stack

Document the versions and tools actually used by the team.

- **Language:** C++
- **Build system:** CMake
- **Compiler:** GCC, Clang, or MSVC, depending on the platform
- **Version control:** Git
- **Repository hosting:** GitHub
- **Testing:** The test framework or testing mechanism adopted by the team
- **Continuous integration:** GitHub Actions, if configured

## 6. Project Architecture

Explain the purpose of each directory and how the modules interact.

```text
DataTable/
├── README.md
├── CMakeLists.txt
├── LICENSE
├── .gitignore
├── include/
│   └── datatable/
├── src/
├── tests/
├── examples/
├── reports/
│   ├── week-01.md
│   ├── week-02.md
│   ├── week-03.md
│   ├── week-04.md
│   └── final-report.md
├── data/
│   ├── input/
│   └── output/
└── .github/
    └── workflows/
        └── build.yml
```

Keep this tree synchronized with the repository. Remove entries for files or directories that do not exist.

## 7. Requirements

List the supported operating systems, a compatible C++ compiler, the required C++ language standard, CMake version, and any additional dependencies.

## 8. Installation and Build

### Clone the repository

```bash
git clone <repository-url>
cd DataTable
```

### Configure

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build
```

### Run the tests

```bash
ctest --test-dir build --output-on-failure
```

### Installation

After configuring an install target in CMake, document the verified installation command for each supported platform.

**Important:** Test every command on a clean checkout before documenting it as supported.

## 9. Usage

Explain how another C++ project can include the public headers, link against the DataTable library, create a table, manipulate data, and export results.

Insert a short, working C++ example after the public API has stabilized. Do not document functions that have not been implemented.

## 10. Testing

Document:

- Unit tests for individual modules.
- Integration tests for interactions between modules.
- Normal, boundary, empty-input, and invalid-input cases.
- CSV parsing and export tests.
- Sorting, filtering, grouping, and aggregation tests.
- Build and test commands.
- The actual test results and any known failures.

## 11. Performance and Design Considerations

Describe the data structures and algorithms selected by the team, the reasons for those decisions, and how unnecessary copying, repeated parsing, and avoidable allocations are minimized.

Include measured performance results only when benchmarks have actually been performed.

## 12. Limitations

List the project's current limitations honestly. Possible areas to discuss include supported data types, CSV parsing constraints, missing-value handling, memory usage, and unsupported file formats.

Do not describe planned features as completed functionality.

## 13. Contributing

Explain the group's development workflow:

1. Work on the assigned feature branch.
2. Synchronize with `develop` when needed.
3. Implement changes and run relevant tests.
4. Commit using clear, descriptive messages.
5. Push the feature branch to GitHub.
6. Open a pull request targeting `develop`.
7. Address review feedback and obtain approval before merging.
8. Keep `main` stable and submission-ready.

## 14. Team Members

| No. | Name | Registration Number |
|---:|---|---|
| 1 | Mukhaye Primah | 25/U/0840 |
| 2 | Manige Daniel | 25/U/08745/PS |
| 3 | Nantale Swabrah | 25/U/08772/PS |
| 4 | Alele Brian Jesse | 25/U/26714 |
| 5 | Tushabe Paul | 25/U/30756 |
| 6 | Bifandhuba Elisha Alvin | 25/U/08704/PS |
| 7 | Waran Newton Mono Korsuk | 25/X/08621/PS |
| 8 | Mwanje Hamuza Tendo | 25/U/05654/PS |

**Programme:** BCCE

## 15. AI Use Declaration

Document AI tools used during development, their purposes, the relevant team members' use, and how any suggestions were verified. Follow the project's academic instructions and ensure this declaration accurately reflects the team's work.

## 16. License

Specify the project's chosen license and link to the `LICENSE` file. Do not claim a license has been selected until the team agrees on one.
