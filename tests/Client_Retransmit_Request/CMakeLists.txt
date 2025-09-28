cmake_minimum_required(VERSION 3.10)
project(BitStreamClient)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Optimization flag -O2
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Sources
set(SOURCES
    client.cpp
    ../../src/Protocol/market_data.pb.cc
)

# Find Protobuf + Abseil
find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/tests
)

# Create executable
add_executable(client ${SOURCES})

# Link libraries
target_link_libraries(client
    ${Protobuf_LIBRARIES}
    absl::log_initialize
    absl::log_internal_check_op
    absl::log_internal_message
    absl::strings
    absl::base
    absl::raw_logging_internal
    absl::log_severity
    ws2_32
    bcrypt
    dbghelp
)
