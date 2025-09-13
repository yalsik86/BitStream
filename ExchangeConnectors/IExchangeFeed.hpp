#pragma once

#include "../Structs/ExchangeUpdate.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <optional>
#include <thread>
#include <chrono>
#include <stop_token>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
using tcp = asio::ip::tcp;

class IExchangeFeed {
  public:
    virtual ~IExchangeFeed() = default;

    virtual void run(std::stop_token) = 0;
    virtual void connect() = 0;
    virtual void receiveUpdates(std::stop_token) = 0;
    virtual std::optional<ExchangeUpdate> parseRaw(const std::string&) = 0;
    virtual void disconnect() = 0;
};