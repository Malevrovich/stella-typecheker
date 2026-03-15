add_executable(stella-ast-tests
    attribute_storage_test.cpp
)

target_include_directories(stella-ast-tests PRIVATE ../include)
target_link_libraries(stella-ast-tests PRIVATE stella-ast gtest gtest_main)

add_test(stella_ast_tests COMMAND stella-ast-tests)