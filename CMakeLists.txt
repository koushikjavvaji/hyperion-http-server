cmake_minimum_required(VERSION 3.20)
project(hyperion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release build = full optimizations
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(hyperion ${SOURCES})