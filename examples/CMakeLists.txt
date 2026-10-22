add_executable(basic_table
    basic_table.cpp
)

target_link_libraries(basic_table
    PRIVATE
        DataTable::DataTable
)

# Load and analyze an external CSV file supplied on the command line.
add_executable(dataset_example
    dataset_example.cpp
)

target_link_libraries(dataset_example
    PRIVATE
        DataTable::DataTable
)

# Build examples that exercise the public API and provide release smoke tests.
foreach(example IN ITEMS csv filtering grouping export integration performance)
    add_executable(${example}_example
        ${example}_example.cpp
    )

    target_link_libraries(${example}_example
        PRIVATE
            DataTable::DataTable
    )
endforeach()

# Run every public example during CTest when both build options are enabled.
if(DATATABLE_BUILD_TESTS)
    add_test(
        NAME DataTable.Example.basic_table
        COMMAND basic_table
    )

    foreach(example IN ITEMS csv filtering grouping export integration performance)
        add_test(
            NAME DataTable.Example.${example}
            COMMAND ${example}_example
        )
    endforeach()
endif()