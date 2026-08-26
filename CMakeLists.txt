cmake_minimum_required(VERSION 3.16)

project(IMPatterns LANGUAGES CXX VERSION 0.2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ASSETS_DIR  "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_LINK "${CMAKE_BINARY_DIR}/assets")
file (CREATE_LINK ${ASSETS_DIR} ${ASSETS_LINK} SYMBOLIC)

if (MSVC)
    message(WARNING "MSVC produced executable is much slower than clangs for some reason, try fiddling with compiler flags")
    set(CXX_EXTRA_FLAGS "/W4" "/WX" "/Zc:__cplusplus" "/wd4324")
else()
    set(CXX_EXTRA_FLAGS "-Wall" "-Wextra" "-Wconversion" "-Werror")
endif()

set(LIBRARY_SOURCES
    "${CMAKE_SOURCE_DIR}/source/mmap_windows.cpp"
    "${CMAKE_SOURCE_DIR}/source/mmap_posix.cpp")
set(LIBRARY_HEADERS
    "${CMAKE_SOURCE_DIR}/include/FixedString.hpp"
    "${CMAKE_SOURCE_DIR}/include/MappedFile.hpp"
    "${CMAKE_SOURCE_DIR}/include/MemStream.hpp"
    "${CMAKE_SOURCE_DIR}/include/Patterns.hpp"
    "${CMAKE_SOURCE_DIR}/include/Match.hpp")

add_library(IMPatterns
    STATIC  ${LIBRARY_SOURCES})
target_compile_options(IMPatterns
    PRIVATE ${CXX_EXTRA_FLAGS})
target_include_directories(IMPatterns
    PRIVATE "include/")
set_target_properties(IMPatterns
    PROPERTIES
        OUTPUT_NAME         "impatterns"
        OUTPUT_NAME_DEBUG   "impatterns-debug")

if (NOT WIN32)
install(
    FILES       ${LIBRARY_HEADERS}
    DESTINATION "include/IMPatterns"
    COMPONENT   imp-headers)
install(
    TARGETS     IMPatterns
    DESTINATION "lib/"
    COMPONENT   imp-binaries)

include(CPack)

set(CPACK_PACKAGE_NAME "IMPatterns")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCTIPTION_SUMMARY "an immediate mode text-pattern recognition library, heavily influenced by LPeg, reworked from classy-patterns")
set(CPACK_INSTALL_PREFIX "/usr/local/")

cpack_add_component(imp-headers
    DISPLAY "Header files")
cpack_add_component(imp-binaries
    DISPLAY "Binary files")
endif()

add_executable(tests
    "tests/JSON.cpp")
target_include_directories(tests
    PRIVATE "include/")
target_compile_options(tests
    PRIVATE ${CXX_EXTRA_FLAGS})
target_link_libraries(tests
    IMPatterns)

add_executable(make_json
    "tests/MakeJSON.cpp")
target_include_directories(tests
    PRIVATE "include/")
target_compile_options(tests
    PRIVATE ${CXX_EXTRA_FLAGS})