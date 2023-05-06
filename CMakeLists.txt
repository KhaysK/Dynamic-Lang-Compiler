cmake_minimum_required(VERSION 3.16)
project(nonamelang)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES
    lexer.cpp
    ast.cpp
    MemoryKernel.cpp
)

find_package(BISON)
BISON_TARGET(parser parser.ypp ${CMAKE_SOURCE_DIR}/parser.tab.cpp
             DEFINES_FILE ${CMAKE_SOURCE_DIR}/parser.tab.hpp)

add_compile_options(-Wno-c99-designator -ggdb -O0)
add_executable(compiler ${SOURCE_FILES} ${BISON_parser_OUTPUTS})
