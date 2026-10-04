cmake_minimum_required(VERSION 3.20)

project(DataTable
    VERSION 1.0.0
    DESCRIPTION "High-performance C++ data table formatting and presentation library"
    LANGUAGES CXX
)

# ============================================================
# C++ STANDARD
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# BUILD OPTIONS
# ============================================================

option(DATATABLE_BUILD_TESTS
       "Build DataTable tests"
       ON)

option(DATATABLE_BUILD_EXAMPLES
       "Build DataTable examples"
       ON)

# ============================================================
# LIBRARY
# ============================================================

add_library(DataTable
    src/table.cpp
    src/column.cpp
    src/row.cpp
    src/cell.cpp
    src/csv.cpp
    src/sort.cpp
    src/filter.cpp
    src/group.cpp
    src/aggregate.cpp
    src/formatter.cpp
    src/style.cpp
    src/export.cpp
)

# ============================================================
# PUBLIC INCLUDE DIRECTORY
# ============================================================

target_include_directories(DataTable
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ============================================================
# COMPILER WARNINGS
# ============================================================

if(MSVC)

    target_compile_options(DataTable PRIVATE
        /W4
        /permissive-
    )

else()

    target_compile_options(DataTable PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()

# ============================================================
# ALIAS FOR CONSUMERS
# ============================================================

add_library(DataTable::DataTable ALIAS DataTable)

# ============================================================
# TESTING
# ============================================================

if(DATATABLE_BUILD_TESTS)

    enable_testing()

    add_subdirectory(tests)

endif()

# ============================================================
# EXAMPLES
# ============================================================

if(DATATABLE_BUILD_EXAMPLES)

    add_subdirectory(examples)

endif()

# ============================================================
# INSTALLATION
# ============================================================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install library
install(
    TARGETS DataTable
    EXPORT DataTableTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install public headers
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(
    EXPORT DataTableTargets
    FILE DataTableTargets.cmake
    NAMESPACE DataTable::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DataTable
)

# Generate package configuration
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DataTableConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DataTableConfig.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/DataTable
)

# Generate version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/DataTableConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install configuration files
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/DataTableConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/DataTableConfigVersion.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/DataTable
)