set(AST_SOURCES src/base.cpp src/fun.cpp src/logic.cpp src/nat.cpp)

add_library(stella-ast ${AST_SOURCES})
target_include_directories(stella-ast PUBLIC include)
target_link_libraries(stella-ast PRIVATE loguru::loguru)
