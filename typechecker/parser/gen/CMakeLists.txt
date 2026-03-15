list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/antlr4)

# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

set(ANTLR4_ZIP_REPOSITORY
    https://github.com/antlr/antlr4/archive/refs/tags/4.13.2.zip)

include(ExternalAntlr4Cpp)
include_directories(${ANTLR4_INCLUDE_DIRS})

set(ANTLR_EXECUTABLE
    ${CMAKE_SOURCE_DIR}/thirdparty/antlr4/antlr-4.13.2-complete.jar)
find_package(ANTLR REQUIRED)

antlr_target(
  StellaGrammarLexer
  ${CMAKE_CURRENT_SOURCE_DIR}/stella-grammar/StellaLexer.g4
  LEXER
  PACKAGE
  antlr4_stella
  OUTPUT_DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR}/antlr4-output)
antlr_target(
  StellaGrammarParser
  ${CMAKE_CURRENT_SOURCE_DIR}/stella-grammar/StellaParser.g4
  PARSER
  VISITOR
  PACKAGE
  antlr4_stella
  OUTPUT_DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR}/antlr4-output
  DEPENDS_ANTLR
  StellaGrammarLexer
  COMPILE_FLAGS
  -lib
  ${ANTLR_StellaGrammarLexer_OUTPUT_DIR})

include_directories(${ANTLR_StellaGrammarLexer_OUTPUT_DIR})
include_directories(${ANTLR_StellaGrammarParser_OUTPUT_DIR})

add_library(antlr4-stella STATIC ${ANTLR_StellaGrammarLexer_CXX_OUTPUTS}
                                 ${ANTLR_StellaGrammarParser_CXX_OUTPUTS})

target_link_libraries(antlr4-stella antlr4_static)
target_include_directories(
  antlr4-stella INTERFACE ${ANTLR_StellaGrammarLexer_OUTPUT_DIR}
                          ${ANTLR_StellaGrammarParser_OUTPUT_DIR})

add_executable(parse-tree-printer parse_tree_printer.cpp)
target_link_libraries(parse-tree-printer antlr4-stella)
