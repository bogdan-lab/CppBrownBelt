cmake_minimum_required(VERSION 3.0)

PROJECT(Polynomial)


set (CMAKE_VERBOSE_MAKEFILE ON)

set (CMAKE_CXX_STANDARD   17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -g -Wall  -Wextra -pedantic -Wno-sign-compare" )

add_executable( poly
    polynomial.cpp
)

target_include_directories(poly PUBLIC ../)

install(TARGETS poly  RUNTIME DESTINATION ${Polynomial_SOURCE_DIR}/bin)
