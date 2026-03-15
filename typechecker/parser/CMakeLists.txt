add_subdirectory(gen)

set(PARSER_SOURCES src/parser.cpp)

add_library(stella-parser ${PARSER_SOURCES})
target_include_directories(stella-parser PUBLIC include)
target_link_libraries(stella-parser PUBLIC stella-ast antlr4-stella
                                           loguru::loguru)
