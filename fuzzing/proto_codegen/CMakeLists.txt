# ─── fuzzing/proto_codegen/CMakeLists.txt ────────────────────────────────────
#
# Builds the static library  stella-proto-codegen  which provides:
#   • the protobuf-generated AST messages  (ast.pb.h / ast.pb.cc)
#   • the Stella source-code serialiser    (proto_to_stella.h / .cpp)
#
# Protobuf is resolved in this order:
#   1. System / user-installed package  (find_package CONFIG)
#   2. Legacy FindProtobuf module       (find_package MODULE)
#   3. Automatic download via FetchContent
#
# NOTE ON ABSEIL VERSIONS
#   protobuf 27+ and fuzztest (main) both require abseil 20240116.2.
#   We declare abseil once here (before protobuf) so CMake deduplicates it
#   when fuzz_parser also calls FetchContent_MakeAvailable(abseil-cpp).
#
# Consumers only need:
#   target_link_libraries(my_target PRIVATE stella-proto-codegen)
# ─────────────────────────────────────────────────────────────────────────────

cmake_minimum_required(VERSION 3.14)

# ── 1. Try to find an already-installed protobuf ──────────────────────────────
find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
    find_package(Protobuf QUIET)   # legacy module
endif()

# ── 2. If not found, download it with FetchContent ────────────────────────────
if(NOT Protobuf_FOUND AND NOT TARGET protobuf::libprotobuf)
    message(STATUS "[proto_codegen] Protobuf not found locally — fetching via FetchContent")

    include(FetchContent)

    # ── 2a. Abseil — declare FIRST so protobuf and fuzztest share one copy.
    #        Version 20240116.2 is required by both protobuf 27.x and fuzztest.
    if(NOT TARGET absl::base)
        FetchContent_Declare(
            abseil-cpp
            GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
            GIT_TAG        20240722.0
            GIT_SHALLOW    TRUE
        )
        set(ABSL_PROPAGATE_CXX_STD ON  CACHE BOOL "" FORCE)
        set(ABSL_ENABLE_INSTALL    OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(abseil-cpp)
    endif()

    # ── 2b. Protobuf 27.3 — compatible with abseil 20240116.2.
    #        Tell protobuf to use the abseil we already fetched ("package" mode).
    set(protobuf_BUILD_TESTS           OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_CONFORMANCE     OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_EXAMPLES        OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_PROTOC_BINARIES ON  CACHE BOOL "" FORCE)
    set(protobuf_INSTALL               OFF CACHE BOOL "" FORCE)
    set(protobuf_WITH_ZLIB             OFF CACHE BOOL "" FORCE)
    set(protobuf_ABSL_PROVIDER         "package" CACHE STRING "" FORCE)

    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v27.3
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(protobuf)

    set(Protobuf_FOUND TRUE)
endif()

# ── 3. Generate C++ sources from ast.proto ────────────────────────────────────
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proto/ast.proto")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

# ── 4. Static library ─────────────────────────────────────────────────────────
add_library(stella-proto-codegen STATIC
    ${PROTO_SRCS}
    src/proto_to_stella.cpp
)

target_include_directories(stella-proto-codegen
    PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}"          # for ast.pb.h
        "${CMAKE_CURRENT_SOURCE_DIR}/include"  # for stella/fuzzing/proto_to_stella.h
)

if(TARGET protobuf::libprotobuf)
    target_link_libraries(stella-proto-codegen PUBLIC protobuf::libprotobuf)
else()
    target_link_libraries(stella-proto-codegen PUBLIC ${PROTOBUF_LIBRARIES})
    target_include_directories(stella-proto-codegen PUBLIC ${PROTOBUF_INCLUDE_DIRS})
endif()

target_compile_options(stella-proto-codegen PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-deprecated-declarations>
)
