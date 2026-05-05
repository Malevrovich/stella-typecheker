# ─── fuzzing/fuzz_typechecker/CMakeLists.txt ─────────────────────────────────
#
# Fuzz target: proto AST → Stella source string → parse → typecheck.
#
# Property under test:
#   The type-checker must never crash (segfault, abort, std::terminate, or
#   unhandled exception) regardless of the semantic content of the program.
#   TypeCheckError is expected and silently swallowed.
#
# Options:
#   -DFUZZ_TYPECHECKER=ON     (default OFF — skips this target in normal CI)
#   -DFUZZ_ENGINE=ON          (default OFF — enables libFuzzer/ASan fuzzing mode)
#
# Unit-test mode (no special toolchain needed):
#   cmake -S . -B build -DFUZZ_TYPECHECKER=ON
#   cmake --build build --target fuzz_typechecker
#   ./build/fuzzing/fuzz_typechecker/fuzz_typechecker --gtest_filter=*
#
# Real fuzzing mode (requires clang + libFuzzer + ASan):
#   cmake -S . -B build \
#         -DCMAKE_CXX_COMPILER=clang++ \
#         -DFUZZ_TYPECHECKER=ON -DFUZZ_ENGINE=ON \
#         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
#         -DCMAKE_CXX_FLAGS="-fsanitize=address,fuzzer-no-link"
#   cmake --build build --target fuzz_typechecker
#   ./build/fuzzing/fuzz_typechecker/fuzz_typechecker \
#       --fuzz=StellaTypecheckerFuzz.TypecheckDoesNotCrash
# ─────────────────────────────────────────────────────────────────────────────

option(FUZZ_TYPECHECKER "Build the structure-aware type-checker fuzz target" OFF)
# FUZZ_ENGINE is declared in fuzz_parser/CMakeLists.txt; guard against
# re-declaration if both targets are enabled in the same build.
if(NOT DEFINED FUZZ_ENGINE)
    option(FUZZ_ENGINE "Enable libFuzzer/ASan fuzzing engine (requires clang + ASan)" OFF)
endif()

if(NOT FUZZ_TYPECHECKER)
    message(STATUS "[fuzz_typechecker] Skipped (pass -DFUZZ_TYPECHECKER=ON to enable)")
    return()
endif()

include(FetchContent)

# ── 1. Google FuzzTest (and its dependencies) ─────────────────────────────────
# FetchContent deduplicates by name: if fuzz_parser already populated these,
# the declarations below are no-ops.  We still declare them so that
# fuzz_typechecker can be built standalone (without -DFUZZ_PARSER=ON).

if(NOT TARGET absl::base)
    FetchContent_Declare(
        abseil-cpp
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG        20240722.0
        GIT_SHALLOW    TRUE
    )
    set(ABSL_PROPAGATE_CXX_STD ON  CACHE BOOL "" FORCE)
    set(ABSL_ENABLE_INSTALL    OFF CACHE BOOL "" FORCE)
    set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR} CACHE STRING "" FORCE)
    FetchContent_MakeAvailable(abseil-cpp)
endif()

# re2 — transitive dependency of fuzztest.
# Declare before fuzztest so we can suppress its install rules.
FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG        2024-07-02
    GIT_SHALLOW    TRUE
)
if(NOT TARGET re2::re2)
    set(RE2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "re2_dummy" CACHE STRING "" FORCE)
    FetchContent_MakeAvailable(re2)
endif()
# Ensure re2_SOURCE_DIR is populated even if re2 was already made available
# by a sibling subdirectory (fuzz_parser).
FetchContent_GetProperties(re2)

# FuzzTest — 2025-02-14 tag fixes arm64/Apple Silicon -msse4.1 issue.
if(NOT TARGET fuzztest::fuzztest)
    FetchContent_Declare(
        fuzztest
        GIT_REPOSITORY https://github.com/google/fuzztest.git
        GIT_TAG        2025-02-14
        GIT_SHALLOW    TRUE
    )
    if(FUZZ_ENGINE)
        set(FUZZTEST_FUZZING_MODE ON CACHE BOOL "" FORCE)
    else()
        set(FUZZTEST_FUZZING_MODE OFF CACHE BOOL "" FORCE)
    endif()
    FetchContent_MakeAvailable(fuzztest)
endif()

# ── 2. Fuzz target executable ─────────────────────────────────────────────────
add_executable(fuzz_typechecker fuzz_typechecker.cpp)

target_link_libraries(fuzz_typechecker
    PRIVATE
        stella-proto-codegen          # ast.pb.h + proto_to_stella.h
        stella-parser                 # ParseProgramText()
        stella-type-check             # CheckProgram() + TypeCheckError
        loguru::loguru                # loguru::set_fatal_handler()
        fuzztest::fuzztest_gtest_main # FuzzTest + GTest main
)

# FuzzTest uses include_directories(${re2_SOURCE_DIR}) in its own scope, but
# that doesn't propagate to targets defined outside FuzzTest's CMakeLists.
if(DEFINED re2_SOURCE_DIR)
    target_include_directories(fuzz_typechecker PRIVATE ${re2_SOURCE_DIR})
endif()

# FuzzTest requires C++17 minimum; the project already uses C++20.
target_compile_features(fuzz_typechecker PRIVATE cxx_std_20)

# Register with CTest so `ctest` runs the fuzz target in unit-test mode.
include(GoogleTest)
gtest_discover_tests(fuzz_typechecker)
