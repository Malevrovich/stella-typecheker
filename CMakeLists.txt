cmake_minimum_required(VERSION 3.14)
project(stella-typechecker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include(FetchContent)

FetchContent_Declare(
  LoguruGitRepo
  GIT_REPOSITORY "https://github.com/emilk/loguru"
  GIT_TAG "master")
set(LOGURU_WITH_STREAMS TRUE)
FetchContent_MakeAvailable(LoguruGitRepo)

add_compile_definitions(LOGURU_WITH_STREAMS=1)

FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG v0.9.6)
FetchContent_MakeAvailable(magic_enum)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(typechecker)
add_subdirectory(fuzzing)