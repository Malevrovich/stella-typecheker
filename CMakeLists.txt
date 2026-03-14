cmake_minimum_required(VERSION 3.10)
project(stella-typechecker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_subdirectory(typechecker)
add_subdirectory(tests)
