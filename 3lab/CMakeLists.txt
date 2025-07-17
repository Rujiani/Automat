cmake_minimum_required(VERSION 3.14)
project(robot C)

set(CMAKE_C_STANDARD 99)

enable_testing()
find_package(BISON REQUIRED)
find_package(FLEX  REQUIRED)

BISON_TARGET(Parser parser.y ${CMAKE_BINARY_DIR}/parser.c
             DEFINES_FILE    ${CMAKE_BINARY_DIR}/parser.h
             COMPILE_FLAGS "-v -Wnone --debug" )

FLEX_TARGET(Lexer lexer.l   ${CMAKE_BINARY_DIR}/lex.yy.c)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(robot
    ${BISON_Parser_OUTPUTS}   # parser.c
    ${FLEX_Lexer_OUTPUTS}     # lex.yy.c
    values.c
    variables.c
    actions.c
    gc.c
    field.c
    maze.c
    robot.c
    func.c)


target_compile_options(robot PRIVATE -Wall -Wextra -D_GNU_SOURCE)
add_subdirectory(tests)