project(json_parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ASSETS_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(ASSETS_LINK "${CMAKE_CURRENT_BINARY_DIR}/assets")
file (CREATE_LINK ${ASSETS_DIR} ${ASSETS_LINK} SYMBOLIC)

add_executable(json_parser
    "source/JSON.cpp")
target_include_directories(json_parser
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        "${CMAKE_SOURCE_DIR}/include")
target_compile_options(json_parser
    PRIVATE ${CXX_EXTRA_FLAGS})
target_link_libraries(json_parser
    IMPatterns)

add_executable(make_json
    "source/MakeJSON.cpp")
target_include_directories(make_json
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        "${CMAKE_SOURCE_DIR}/include")
target_compile_options(make_json
    PRIVATE ${CXX_EXTRA_FLAGS})

if (MSVC) # using 'unsafe' fopen instead of un-portable fopen_s
    target_compile_definitions(make_json
        PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()