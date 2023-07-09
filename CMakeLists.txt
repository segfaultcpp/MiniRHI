cmake_minimum_required(VERSION 3.20)
project(MiniRHI VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(MINIRHI_3RDPARTY_DIR "${CMAKE_SOURCE_DIR}/third_party/")

set(CMAKE_CXX_FLAGS "-fsanitize=undefined -MT")
add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)

add_subdirectory(lib)
add_subdirectory(test)