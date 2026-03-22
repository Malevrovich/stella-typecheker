add_executable(stella-checker-tests
    expected_type_test.cpp
)

target_include_directories(stella-checker-tests PRIVATE ../include)
target_link_libraries(stella-checker-tests PRIVATE stella-type-check gtest gtest_main)

add_test(stella_checker_tests COMMAND stella-checker-tests)
