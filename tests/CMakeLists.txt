include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(test_low test_low.c)
add_dependencies(test_low test_low)
add_test(test_low test_low)
target_link_libraries(test_low low util cmocka "-Wl,--wrap=get_StringFromSysFile -Wl,--wrap=readdir")

add_executable(test_util test_util.c)
add_dependencies(test_util test_util)
add_test(test_util test_util)
target_link_libraries(test_util util cmocka)