add_executable(basic_table
    basic_table.cpp
)

target_link_libraries(basic_table
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