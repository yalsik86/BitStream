cmake_minimum_required(VERSION 3.10)
project(StructsTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Optimization flag -O2
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Sources
set(SOURCES
    structs_test.cpp
)

# Create executable
add_executable(structs_test ${SOURCES})

# include directories
target_include_directories(structs_test PRIVATE
    ../../include
)