project(expression_parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ASSETS_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(ASSETS_LINK "${CMAKE_CURRENT_BINARY_DIR}/assets")
file (CREATE_LINK ${ASSETS_DIR} ${ASSETS_LINK} SYMBOLIC)

set(CXX_WARNINGS "-Wall" "-Wextra" "-Wconversion" "-Wnrvo" "-Werror")

add_executable(expression_parser
    "source/main.cpp")
target_include_directories(expression_parser
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_compile_options(expression_parser
    PRIVATE ${CXX_WARNINGS})
target_link_libraries(expression_parser
    PRIVATE impatterns)