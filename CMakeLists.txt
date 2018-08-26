cmake_minimum_required(VERSION 2.8)
project(keryx)

# compilation options
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic -Wextra -Werror --std=c++1y -fdiagnostics-color") 
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -DNDEBUG -O3")

# lib
file(GLOB_RECURSE LIB_SOURCES src/lib/*.cpp)
add_library(libkeryx SHARED ${LIB_SOURCES})

# test executable
file(GLOB EXE_SOURCES "src/*.cpp")
add_executable(keryx ${EXE_SOURCES})
target_include_directories(keryx PUBLIC ${CMAKE_SOURCE_DIR}/src/lib )
target_link_libraries(keryx libkeryx)

# helpers to make it safe to use globing to define files in build
include(tools/safe_dep_glob.cmake)
update_deps_file("${LIB_SOURCES};${EXE_SOURCES}")

