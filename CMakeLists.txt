cmake_minimum_required(VERSION 3.16)

project(IMPatterns LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ASSETS_DIR  "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_LINK "${CMAKE_BINARY_DIR}/assets")
file (CREATE_LINK ${ASSETS_DIR} ${ASSETS_LINK} SYMBOLIC)

include(CPack)

set(CPACK_PACKAGE_NAME "IMPatterns")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCTIPTION_SUMMARY "an immediate mode text-pattern recognition library, heavily influenced by LPeg, reworked from classy-patterns")
set(CPACK_INSTALL_PREFIX "/usr/local/")

set(LIBRARY_HEADERS
    "${CMAKE_SOURCE_DIR}/include/FixedString.hpp"
    "${CMAKE_SOURCE_DIR}/include/Patterns.hpp"
    "${CMAKE_SOURCE_DIR}/include/Match.hpp")
add_library(IMPatterns
    INTERFACE ${LIBRARY_HEADERS})
install(
    FILES       ${LIBRARY_HEADERS}
    DESTINATION "include/IMPatterns"
    COMPONENT   imp-headers)
cpack_add_component(imp-headers
    DISPLAY "Header files")

set(CXX_WARNINGS "-Wall" "-Wextra" "-Wconversion" "-Werror")

add_executable(tests
    "tests/JSON.cpp")
target_include_directories(tests
    PRIVATE "include/")
target_compile_options(tests
    PRIVATE ${CXX_WARNINGS})
target_compile_options(tests
    PRIVATE $<$<CONFIG:Release>:-march=native>)

add_executable(make_json
    "tests/MakeJSON.cpp")
target_include_directories(tests
    PRIVATE "include/")
target_compile_options(tests
    PRIVATE ${CXX_WARNINGS})
target_compile_options(tests
    PRIVATE $<$<CONFIG:Release>:-march=native>)