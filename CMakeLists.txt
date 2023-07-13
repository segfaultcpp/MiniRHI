cmake_minimum_required(VERSION 3.20)

set(ANDROID_ENABLE YES)

if (${ANDROID_ENABLE})
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)

    set(ANDROID_NDK_HOME $ENV{ANDROID_NDK_HOME})
    if(MSVC)
        fatal(You must use clang when building for android)
    endif()

    set(CMAKE_CXX_COMPILER "${ANDROID_NDK_HOME}toolchains/llvm/prebuilt/windows-x86_64/bin/aarch64-linux-android30-clang.cmd")
    add_compile_options(--target=aarch64-linux-android30)
    add_definitions(-DANDROID)
endif()

if (MINIRHI_ENABLE_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-p=${CMAKE_BINARY_DIR};-format-style=file;--use-color")
endif()

cmake_policy(SET CMP0091 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(MINIRHI_3RDPARTY_DIR "${CMAKE_SOURCE_DIR}/third_party/")

project(MiniRHI LANGUAGES CXX VERSION 0.1)

add_subdirectory(lib)
add_subdirectory(test)

if(${ANDROID_ENABLE})
    add_subdirectory(android)
endif()
