cmake_minimum_required(VERSION 3.10)
project(BitStream)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Optimization flag -O1
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")

# Sources
set(SOURCES
    main.cpp
    ExchangeConnectors/CoinbaseFeed.cpp
    ExchangeConnectors/BinanceFeed.cpp
    ExchangeConnectors/KrakenFeed.cpp
    ExchangeConnectors/CryptoComFeed.cpp
    AggregatorEngine/AggregatorEngine.cpp
    MarketDataRouter/MarketDataRouter.cpp
    Protocol/market_data.pb.cc
)

# Find required packages
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
find_package(absl REQUIRED)

# Include directories
include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create executable
add_executable(routertest ${SOURCES})

# Link libraries
target_link_libraries(routertest
    ${Protobuf_LIBRARIES}
    ${OPENSSL_LIBRARIES}
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