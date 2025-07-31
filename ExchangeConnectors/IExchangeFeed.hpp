#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class IExchangeFeed {
  public:
    virtual ~IExchangeFeed() = default;

    virtual void connect() = 0;
};