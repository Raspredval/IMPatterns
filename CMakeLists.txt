cmake_minimum_required(VERSION 3.30)

project(IMPatterns LANGUAGES CXX VERSION 0.2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC) # msvc or clang-cl
    message(FATAL_ERROR "MSVC has the most dumbest bugs and most non-compliant standard support ever, not supported")
endif()

set(LIBRARY_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/FixedString.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Dictionary.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/SizedTypes.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/MappedFile.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/MemStream.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Patterns.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/CharSet.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Vector.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Match.hpp")

find_package(Boost REQUIRED)

add_library(impatterns INTERFACE)
target_link_libraries(impatterns
    INTERFACE Boost::headers)
target_sources(impatterns
    INTERFACE
        FILE_SET HEADERS
            BASE_DIRS   "${CMAKE_CURRENT_SOURCE_DIR}/include"
            FILES       ${LIBRARY_HEADERS})

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