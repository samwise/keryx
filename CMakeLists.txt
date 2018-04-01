cmake_minimum_required(VERSION 2.8)
project(libkeryx)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++1y" )

file(GLOB SOURCES "src/*.cpp")
add_executable(libkeryx ${SOURCES})
