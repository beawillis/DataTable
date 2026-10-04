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