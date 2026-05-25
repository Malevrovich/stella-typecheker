set(AST_SOURCES
    src/asc.cpp
    src/base.cpp
    src/cast.cpp
    src/exception.cpp
    src/fun.cpp
    src/let.cpp
    src/list.cpp
    src/logic.cpp
    src/match.cpp
    src/nat.cpp
    src/panic.cpp
    src/record.cpp
    src/reference.cpp
    src/sequence.cpp
    src/sum.cpp
    src/tuple.cpp
    src/variant.cpp
    src/visitor.cpp)

add_library(stella-ast ${AST_SOURCES})
target_include_directories(stella-ast PUBLIC include)
target_link_libraries(stella-ast PRIVATE loguru::loguru)

add_subdirectory(tests)
