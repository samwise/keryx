cmake_minimum_required(VERSION 2.8)
project(keryx)

set (FLAGS "${CMAKE_CXX_FLAGS_DEBUG}") 
set (FLAGS "${FLAGS} -Wall")
set (FLAGS "${FLAGS} -pedantic")
set (FLAGS "${FLAGS} -Wextra")
set (FLAGS "${FLAGS} -Werror")
set (FLAGS "${FLAGS} --std=c++1y")
set (FLAGS "${FLAGS} -fdiagnostics-color")

set (CMAKE_CXX_FLAGS_DEBUG "${FLAGS}")
set (CMAKE_CXX_FLAGS_RELEASE "${FLAGS} -DNDEBUG -O3")

file(GLOB LIB_SOURCES "src/lib/*.cpp")
add_library(libkeryx SHARED ${LIB_SOURCES})

file(GLOB SOURCES "src/main.cpp")
add_executable(keryx ${SOURCES})
target_link_libraries(keryx libkeryx)
