cmake_minimum_required(VERSION 3.27)
project(compiler)

set(CMAKE_CXX_STANDARD 11)

enable_testing()
find_package(GTest REQUIRED)

add_executable(compiler_lexer_test
        util/util.h
        util/util.c
        lexer/lexer/lexer.h
        lexer/lexer/lexer.c
        lexer/tests/lexer_test.cpp
)

target_include_directories(compiler_lexer_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(compiler_lexer_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

include_directories(
        util
        lexer/lexer
        parser/parser
        parser/ast
        semantic/semantic
)

add_executable(compiler
        util/util.h
        util/util.c
        lexer/lexer/lexer.h
        lexer/lexer/lexer.c
        Driver.cpp
        parser/parser/parser.cpp
        parser/parser/parser.hpp
        semantic/semantic/semantic.cpp
        semantic/semantic/semantic.hpp
        parser/ast/ast.hpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)
target_link_libraries(compiler PkgConfig::deps)

