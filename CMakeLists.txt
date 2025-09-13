cmake_minimum_required(VERSION 3.10)
project(BitStream)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Optimization flag -O1
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")

# Sources
set(SOURCES
    src/main.cpp
    src/FeedHandlers/CoinbaseFeed.cpp
    src/FeedHandlers/BinanceFeed.cpp
    src/FeedHandlers/KrakenFeed.cpp
    src/FeedHandlers/CryptoComFeed.cpp
    src/AggregatorEngine/AggregatorEngine.cpp
    src/DataDisseminator/DataDisseminator.cpp
    src/DataDisseminator/MulticastPublisher.cpp
    src/Protocol/market_data.pb.cc
)

# Find required packages
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
find_package(absl REQUIRED)

# Include directories
include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# Create executable
add_executable(test ${SOURCES})

# Link libraries
target_link_libraries(test
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
    ws2_32
    bcrypt
    dbghelp
)