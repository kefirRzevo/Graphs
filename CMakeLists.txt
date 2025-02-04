cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

add_compile_options (-Werror -Wall -Wextra -Wpedantic)

project(graphs)

add_executable (${PROJECT_NAME} Graphs.cc)
target_include_directories (${PROJECT_NAME} PRIVATE include)
