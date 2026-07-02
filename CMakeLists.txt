cmake_minimum_required(VERSION 3.16)

project(IMPatterns LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ASSETS_DIR  "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_LINK "${CMAKE_BINARY_DIR}/assets")
file (CREATE_LINK ${ASSETS_DIR} ${ASSETS_LINK} SYMBOLIC)

add_executable(IMPatterns
    "tests/JSON.cpp")
target_include_directories(IMPatterns
    PRIVATE "include/")
target_compile_options(IMPatterns
    PRIVATE "-Wall" "-Wextra" "-Wconversion" "-Werror")