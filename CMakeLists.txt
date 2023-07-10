cmake_minimum_required(VERSION 3.20)
project(MiniRHI VERSION 0.1)

if (MINIRHI_ENABLE_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-p=${CMAKE_BINARY_DIR};-format-style=file;--use-color")
endif()

cmake_policy(SET CMP0091 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(MINIRHI_3RDPARTY_DIR "${CMAKE_SOURCE_DIR}/third_party/")

add_subdirectory(lib)
add_subdirectory(test)