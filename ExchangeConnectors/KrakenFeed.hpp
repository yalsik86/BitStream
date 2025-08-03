#pragma once
#include "IExchangeFeed.hpp"

class KrakenFeed : public IExchangeFeed {
  public:
    KrakenFeed();
    void connect() override;
    void readLoop();

  private:
    net::io_context ioc;
    ssl::context ssl_ctx;
    websocket::stream<ssl::stream<tcp::socket>> ws;
};