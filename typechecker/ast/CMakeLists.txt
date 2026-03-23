set(AST_SOURCES src/asc.cpp src/base.cpp src/fun.cpp src/logic.cpp src/nat.cpp src/visitor.cpp)

add_library(stella-ast ${AST_SOURCES})
target_include_directories(stella-ast PUBLIC include)
target_link_libraries(stella-ast PRIVATE loguru::loguru)

add_subdirectory(tests)
