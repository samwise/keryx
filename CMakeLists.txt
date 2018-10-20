cmake_minimum_required(VERSION 2.8)
project(keryx)

# externals
set (EXTERNALS_ROOT "${CMAKE_SOURCE_DIR}/externals/${platform}")
set (EXTERNALS_INCLUDE "${EXTERNALS_ROOT}/include")
set (EXTERNALS_LIB "${EXTERNALS_ROOT}/lib/${CMAKE_BUILD_TYPE}")
include_directories (BEFORE SYSTEM ${EXTERNALS_INCLUDE})
link_directories (${EXTERNALS_LIB})

# compilation options
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} \
  -Wall \
  -pedantic \
  -Wextra \
  -Werror \
  --std=c++17 \
  -fdiagnostics-color") 
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} \
  -DNDEBUG \
  -O3")

# lib
file(GLOB_RECURSE LIB_SOURCES src/lib/*.cpp)
add_library(libkeryx SHARED ${LIB_SOURCES})
target_include_directories(libkeryx PUBLIC ${CMAKE_SOURCE_DIR}/src)

# tesiit executable
file(GLOB EXE_SOURCES "src/*.cpp")
add_executable(keryx ${EXE_SOURCES})
target_include_directories(keryx PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(keryx libkeryx benchmark pthread)

# helpers to make it safe to use globing to define files in build
include(tools/safe_dep_glob.cmake)
update_deps_file("${LIB_SOURCES};${EXE_SOURCES}")

