cmake_minimum_required(VERSION 3.0)

PROJECT(Program)


set (CMAKE_VERBOSE_MAKEFILE ON)

set (CMAKE_CXX_STANDARD   17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -g -Wall  -Wextra -pedantic -Wno-sign-compare" )

add_executable( hash_set
    src/hash_set.cpp
    #src/fem_utils.h
)

#target_include_directories(fem_utils PUBLIC /home/bogdan/workplace/Bibliotech/other/gmsh_sdk/include)
#target_link_libraries(fem_utils /home/bogdan/workplace/Bibliotech/other/gmsh_sdk/lib/libgmsh.so -lm)

install(TARGETS  hash_set  RUNTIME DESTINATION ${Program_SOURCE_DIR}/bin)
