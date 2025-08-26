#pragma once
#include "IExchangeFeed.hpp"

class BinanceFeed : public IExchangeFeed {
  public:
    BinanceFeed();
    void connect() override;
    void receiveUpdates() override;

  private:
    net::io_context ioc;
    ssl::context ssl_ctx;
    websocket::stream<ssl::stream<tcp::socket>> ws;
};