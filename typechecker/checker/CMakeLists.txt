set(TYPE_CHECK_SOURCES
    src/error.cpp
    src/check.cpp
    src/name_context.cpp
    src/type_checker_bool.cpp
    src/type_checker_fun.cpp
    src/type_checker_nat.cpp
    src/type_checker_var.cpp
    src/type_checker.cpp
    src/expected_type.cpp)

add_library(stella-type-check ${TYPE_CHECK_SOURCES})
target_include_directories(stella-type-check PUBLIC include)
target_link_libraries(stella-type-check PUBLIC stella-ast)
target_link_libraries(stella-type-check PRIVATE loguru::loguru
                                                magic_enum::magic_enum)

add_subdirectory(tests)
