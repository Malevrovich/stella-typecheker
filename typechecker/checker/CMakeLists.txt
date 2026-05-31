set(TYPE_CHECK_SOURCES
    src/error.cpp
    src/check.cpp
    src/subtype.cpp
    src/type_checker_asc.cpp
    src/type_checker_auto.cpp
    src/type_checker_cast.cpp
    src/type_checker_bool.cpp
    src/type_checker_exception.cpp
    src/type_checker_fun.cpp
    src/type_checker_generic.cpp
    src/type_checker_let.cpp
    src/type_checker_list.cpp
    src/type_checker_nat.cpp
    src/type_checker_panic.cpp
    src/type_checker_record.cpp
    src/type_checker_match.cpp
    src/type_checker_reference.cpp
    src/type_checker_sequence.cpp
    src/type_checker_sum.cpp
    src/type_checker_top_bottom.cpp
    src/type_checker_tuple.cpp
    src/type_checker_unit.cpp
    src/type_checker_var.cpp
    src/type_checker_variant.cpp
    src/type_checker.cpp
    src/unifier.cpp
    src/expected_type.cpp)

add_library(stella-type-check ${TYPE_CHECK_SOURCES})
target_include_directories(stella-type-check PUBLIC include)
target_link_libraries(stella-type-check PUBLIC stella-ast)
target_link_libraries(stella-type-check PRIVATE loguru::loguru
                                                magic_enum::magic_enum)
