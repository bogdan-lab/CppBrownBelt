cmake_minimum_required(VERSION 3.0)

PROJECT(CollideGeo)


set (CMAKE_VERBOSE_MAKEFILE ON)

set (CMAKE_CXX_STANDARD   17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -g -Wall  -Wextra -pedantic -Wno-sign-compare" )

add_executable( cllgeo
    src/collide.cpp
    src/geo2d.cpp
)

#target_include_directories(fem_utils PUBLIC /home/bogdan/workplace/Bibliotech/other/gmsh_sdk/include)
#target_link_libraries(fem_utils /home/bogdan/workplace/Bibliotech/other/gmsh_sdk/lib/libgmsh.so -lm)

install(TARGETS cllgeo  RUNTIME DESTINATION ${CollideGeo_SOURCE_DIR}/bin)
