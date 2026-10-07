add_executable(test_table
    test_table.cpp
)

target_link_libraries(test_table
    PRIVATE
        DataTable::DataTable
)

add_test(
    NAME DataTable.Table
    COMMAND test_table
)

# Build and register focused tests for each data-model type.
foreach(module IN ITEMS cell column row)
    add_executable(test_${module}
        test_${module}.cpp
    )

    target_link_libraries(test_${module}
        PRIVATE
            DataTable::DataTable
    )

    add_test(
        NAME DataTable.${module}
        COMMAND test_${module}
    )
endforeach()