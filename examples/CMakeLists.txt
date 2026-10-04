add_executable(basic_table
    basic_table.cpp
)

target_link_libraries(basic_table
    PRIVATE
        DataTable::DataTable
)