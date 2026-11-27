cmake_minimum_required(VERSION 3.30)

project(IMPatterns LANGUAGES CXX VERSION 0.2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC) # msvc or clang-cl
    message(FATAL_ERROR "MSVC has the most dumbest bugs and most non-compliant standard support ever, not supported")
else()
    set(CXX_EXTRA_FLAGS "-Wall" "-Wextra" "-Wconversion" "-Wnrvo" "-Werror")
endif()

set(LIBRARY_HEADERS
    "${CMAKE_SOURCE_DIR}/include/FixedString.hpp"
    "${CMAKE_SOURCE_DIR}/include/Dictionary.hpp"
    "${CMAKE_SOURCE_DIR}/include/SizedTypes.hpp"
    "${CMAKE_SOURCE_DIR}/include/MappedFile.hpp"
    "${CMAKE_SOURCE_DIR}/include/MemStream.hpp"
    "${CMAKE_SOURCE_DIR}/include/Patterns.hpp"
    "${CMAKE_SOURCE_DIR}/include/CharSet.hpp"
    "${CMAKE_SOURCE_DIR}/include/Vector.hpp"
    "${CMAKE_SOURCE_DIR}/include/Match.hpp")

if (NOT WIN32)
install(
    FILES       ${LIBRARY_HEADERS}
    DESTINATION "include/IMPatterns"
    COMPONENT   imp-headers)

include(CPack)

set(CPACK_PACKAGE_NAME "IMPatterns")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCTIPTION_SUMMARY "an immediate mode text-pattern recognition library, heavily influenced by LPeg, reworked from classy-patterns")
set(CPACK_INSTALL_PREFIX "/usr/local/")

cpack_add_component(imp-headers
    DISPLAY "Header files")
endif()

add_subdirectory("examples/json_parser/")
add_subdirectory("examples/expression_parser")