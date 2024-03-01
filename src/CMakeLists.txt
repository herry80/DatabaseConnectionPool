aux_source_directory(. SRC_LIST)
aux_source_directory(./test/ TEST_LIST)
add_executable(main ${SRC_LIST} ${TEST_LIST})
target_link_libraries(main mysqlclient pthread)