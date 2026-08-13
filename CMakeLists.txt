cmake_minimum_required(VERSION 3.16)

project(IMPatterns LANGUAGES CXX VERSION 0.2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ASSETS_DIR  "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_LINK "${CMAKE_BINARY_DIR}/assets")
file (CREATE_LINK ${ASSETS_DIR} ${ASSETS_LINK} SYMBOLIC)

if (MSVC)
    set(CXX_WARNINGS "/W4 /WX /WL")
else()
    set(CXX_WARNINGS "-Wall" "-Wextra" "-Wconversion" "-Werror")
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

add_library(IMPatterns_Shared
    SHARED  ${LIBRARY_SOURCES})
target_compile_options(IMPatterns_Shared
    PRIVATE ${CXX_WARNINGS})
target_include_directories(IMPatterns_Shared
    PRIVATE "include/")
set_target_properties(IMPatterns_Shared
    PROPERTIES
        OUTPUT_NAME         "impatterns"
        OUTPUT_NAME_DEBUG   "impatterns-debug")

add_library(IMPatterns_Static
    STATIC  ${LIBRARY_SOURCES})
target_compile_options(IMPatterns_Static
    PRIVATE ${CXX_WARNINGS})
target_include_directories(IMPatterns_Static
    PRIVATE "include/")
set_target_properties(IMPatterns_Static
    PROPERTIES
        OUTPUT_NAME         "impatterns"
        OUTPUT_NAME_DEBUG   "impatterns-debug")

if (WIN32)
install(
    FILES       ${LIBRARY_HEADERS}
    DESTINATION "include/IMPatterns"
    COMPONENT   imp-headers)
install(
    TARGETS     IMPatterns_Shared IMPatterns_Static
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
    PRIVATE ${CXX_WARNINGS})
target_compile_options(tests
    PRIVATE $<$<CONFIG:Release>:-march=native>)
target_link_libraries(tests
    IMPatterns_Shared)

add_executable(make_json
    "tests/MakeJSON.cpp")
target_include_directories(tests
    PRIVATE "include/")
target_compile_options(tests
    PRIVATE ${CXX_WARNINGS})
target_compile_options(tests
    PRIVATE $<$<CONFIG:Release>:-march=native>)