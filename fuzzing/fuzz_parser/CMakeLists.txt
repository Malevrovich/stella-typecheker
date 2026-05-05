# ─── fuzzing/fuzz_parser/CMakeLists.txt ──────────────────────────────────────
#
# Fuzz target: proto AST → Stella source string → ANTLR parse.
#
# Options:
#   -DFUZZ_PARSER=ON          (default OFF — skips this target in normal CI)
#   -DFUZZ_ENGINE=ON          (default OFF — enables libFuzzer/ASan fuzzing mode)
#
# Unit-test mode (no special toolchain needed):
#   cmake -S . -B build -DFUZZ_PARSER=ON
#   cmake --build build --target fuzz_parser
#   ./build/fuzzing/fuzz_parser/fuzz_parser --gtest_filter=*
#
# Real fuzzing mode (requires clang + libFuzzer + ASan):
#   cmake -S . -B build \
#         -DCMAKE_CXX_COMPILER=clang++ \
#         -DFUZZ_PARSER=ON -DFUZZ_ENGINE=ON \
#         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
#         -DCMAKE_CXX_FLAGS="-fsanitize=address,fuzzer-no-link"
#   cmake --build build --target fuzz_parser
#   ./build/fuzzing/fuzz_parser/fuzz_parser --fuzz=StellaParserFuzz.ParseRoundTrip
# ─────────────────────────────────────────────────────────────────────────────

option(FUZZ_PARSER "Build the structure-aware parser fuzz target" OFF)
option(FUZZ_ENGINE "Enable libFuzzer/ASan fuzzing engine (requires clang + ASan)" OFF)

if(NOT FUZZ_PARSER)
    message(STATUS "[fuzz_parser] Skipped (pass -DFUZZ_PARSER=ON to enable)")
    return()
endif()

include(FetchContent)

# ── 1. Google FuzzTest ────────────────────────────────────────────────────────
# abseil-cpp is already declared (and possibly populated) by proto_codegen.
# FetchContent deduplicates by name, so declaring it again here is a no-op
# if proto_codegen already called FetchContent_MakeAvailable(abseil-cpp).
# We still declare it so that if proto_codegen was skipped (system protobuf),
# fuzztest can find abseil targets.
if(NOT TARGET absl::base)
    FetchContent_Declare(
        abseil-cpp
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG        20240722.0
        GIT_SHALLOW    TRUE
    )
    set(ABSL_PROPAGATE_CXX_STD ON  CACHE BOOL "" FORCE)
    set(ABSL_ENABLE_INSTALL    OFF CACHE BOOL "" FORCE)
    # Pass the host system processor so abseil picks the right SIMD backend.
    set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR} CACHE STRING "" FORCE)
    FetchContent_MakeAvailable(abseil-cpp)
endif()

# re2 is a transitive dependency of fuzztest.
# Declare it here BEFORE fuzztest so we can disable its install rules,
# which otherwise fail because abseil was built without install support.
FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG        2024-07-02
    GIT_SHALLOW    TRUE
)
set(RE2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
# Prevent re2 from generating install rules that reference unexported absl targets.
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "re2_dummy" CACHE STRING "" FORCE)
FetchContent_MakeAvailable(re2)

# FuzzTest — 2025-02-14 tag fixes arm64/Apple Silicon -msse4.1 issue.
FetchContent_Declare(
    fuzztest
    GIT_REPOSITORY https://github.com/google/fuzztest.git
    GIT_TAG        2025-02-14
    GIT_SHALLOW    TRUE
)
# FUZZTEST_FUZZING_MODE=ON requires clang + libFuzzer + ASan.
# Keep it OFF for normal Debug/Release builds so the binary runs as a GTest.
# Set -DFUZZ_ENGINE=ON (plus appropriate -fsanitize= flags) for real fuzzing.
if(FUZZ_ENGINE)
    set(FUZZTEST_FUZZING_MODE ON CACHE BOOL "" FORCE)
else()
    set(FUZZTEST_FUZZING_MODE OFF CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(fuzztest)

# ── 2. Fuzz target executable ─────────────────────────────────────────────────
add_executable(fuzz_parser fuzz_parser.cpp)

target_link_libraries(fuzz_parser
    PRIVATE
        stella-proto-codegen          # ast.pb.h + proto_to_stella.h
        antlr4-stella                 # StellaLexer.h + StellaParser.h + antlr4_static
        fuzztest::fuzztest_gtest_main # FuzzTest + GTest main
)

# antlr4-stella already exports its include dirs via INTERFACE, but we also
# need the ANTLR4 runtime headers (ANTLRInputStream.h etc.).
# ExternalAntlr4Cpp sets ANTLR4_INCLUDE_DIRS in the parent scope.
if(DEFINED ANTLR4_INCLUDE_DIRS)
    target_include_directories(fuzz_parser PRIVATE ${ANTLR4_INCLUDE_DIRS})
endif()

# FuzzTest uses include_directories(${re2_SOURCE_DIR}) in its own scope, but
# that doesn't propagate to targets defined outside FuzzTest's CMakeLists.
# We need to add re2's source root explicitly so that fuzztest headers that
# include "re2/re2.h" can be found when compiling fuzz_parser.cpp.
if(DEFINED re2_SOURCE_DIR)
    target_include_directories(fuzz_parser PRIVATE ${re2_SOURCE_DIR})
endif()

# Required by the static ANTLR4 runtime build.
target_compile_definitions(fuzz_parser PRIVATE ANTLR4CPP_STATIC)

# FuzzTest requires C++17 minimum; the project already uses C++20.
target_compile_features(fuzz_parser PRIVATE cxx_std_20)

# Register with CTest so `ctest` runs the fuzz target in unit-test mode.
include(GoogleTest)
gtest_discover_tests(fuzz_parser)
