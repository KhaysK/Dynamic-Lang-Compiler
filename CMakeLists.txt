cmake_minimum_required(VERSION 3.16)
project(nonamelang)

option(DEBUG_MODE "Build in debug mode" OFF)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES
    lexer.cpp
    ast.cpp
    MemoryKernel.cpp
    builtin.cpp
)

find_package(BISON)
BISON_TARGET(parser parser.ypp ${CMAKE_SOURCE_DIR}/parser.tab.cpp
             DEFINES_FILE ${CMAKE_SOURCE_DIR}/parser.tab.hpp)

add_compile_options(-Wno-c99-designator)
if (DEBUG_MODE)
    add_compile_options(-ggdb -O0 -DDEBUG)
else()
    add_compile_options(-O2)
endif()

add_executable(compiler ${SOURCE_FILES} ${BISON_parser_OUTPUTS})
