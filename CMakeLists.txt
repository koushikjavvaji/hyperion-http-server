cmake_minimum_required(VERSION 3.10)

project(HyperionServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include headers
include_directories(include)

# collect all source files
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

# create executable
add_executable(hyperion ${SOURCES})
