cmake_minimum_required(VERSION 3.10)
project(LatencyTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-O3)

# Sources
set(SOURCES
    main.cpp
    TestPublisher.cpp
    TestDisseminator.cpp
    ../../src/Protocol/market_data.pb.cc
    ../../src/DataDisseminator/MulticastPublisher.cpp
    ../../src/DataDisseminator/Retransmitter.cpp
    ../../src/DataDisseminator/DataDisseminator.cpp
)

# Find required packages
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
find_package(absl REQUIRED)

add_executable(Disseminator_latency ${SOURCES})

# Include directories
target_include_directories(Disseminator_latency PRIVATE
    ${Protobuf_INCLUDE_DIRS}
    ../../src
    ../../include
)

# Link libraries
target_link_libraries(Disseminator_latency
    ${Protobuf_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    fmt::fmt
    absl::log_initialize
    absl::log_internal_check_op
    absl::log_internal_message
    absl::strings
    absl::base
    absl::raw_logging_internal
    absl::log_severity
)

# Windows system libraries
if(WIN32)
    target_link_libraries(Disseminator_latency 
        ws2_32 
        bcrypt 
        dbghelp
    )
endif()