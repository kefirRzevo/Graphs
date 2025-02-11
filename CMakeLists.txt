cmake_minimum_required (VERSION 3.16)
project (graphs)

find_package (GTest REQUIRED)
find_package (Boost 1.40 COMPONENTS program_options REQUIRED)

set (CMAKE_CXX_STANDARD 20)
add_compile_options (-O3 -Werror -Wall)

set (ClavcaSources src/ClavcaTest.cpp src/clavca/Options.cpp)
add_executable (Clavca ${ClavcaSources})
target_include_directories (Clavca PRIVATE include)
target_link_libraries(Clavca ${Boost_PROGRAM_OPTIONS_LIBRARY})

enable_testing ()

set (GraphTestSources tests/GraphTest.cpp)
add_executable(GraphTest ${GraphTestSources})
target_include_directories (GraphTest PRIVATE include ${GTEST_INCLUDE_DIRS})
target_link_libraries (GraphTest ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
