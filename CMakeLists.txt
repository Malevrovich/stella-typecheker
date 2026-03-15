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

add_subdirectory(typechecker)
add_subdirectory(tests)
