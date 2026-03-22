set(CMAKE_C_COMPILER clang-18)
set(CMAKE_CXX_COMPILER clang++-18)

set(CMAKE_CXX_FLAGS "-stdlib=libc++" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++ -lc++abi" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "-stdlib=libc++ -lc++abi" CACHE STRING "" FORCE)
