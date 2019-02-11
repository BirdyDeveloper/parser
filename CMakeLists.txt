cmake_minimum_required(VERSION 2.8)

project(parsers)
add_executable(${PROJECT_NAME} main.cpp
                               lexer.cpp lexer.h
                               parser.cpp parser.h
                               grammar.cpp grammar.h
                               utility.cpp utility.h)

set(CMAKE_C_COMPILER /usr/bin/gcc-7)
set(CMAKE_CXX_COMPILER /usr/bin/g++-7)

add_definitions(-Dsrc_folder=CMAKE_CURRENT_SOURCE_DIR)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fsanitize=address  -Wextra -pedantic -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op ")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -Wall -fsanitize=address")
endif()

# create a list of files to copy
set( res
    "tree.css"
    "page_style.css"
    "index.html"
    "treant-js"
    "grammar.txt"
    "grammar2.txt"
    "basic_arithmetic_grammar.txt"
    "basic_arithmetic_grammar2.txt"
    "brackets_grammar.txt"
    "brackets_grammar2.txt"
    "brackets_grammar3.txt"
    "nested_lists_grammar.txt"
    "if_else_grammar.txt"
    "c_function_grammar.txt"
)

# do the copying
foreach(file ${res})
    add_custom_target("config-${file}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${file}" "${CMAKE_CURRENT_BINARY_DIR}/${file}"
      )
    add_dependencies(${PROJECT_NAME} "config-${file}")
endforeach()





