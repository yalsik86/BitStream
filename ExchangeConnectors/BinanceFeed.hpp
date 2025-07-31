#pragma once
#include "IExchangeFeed.hpp"

class BinanceFeed : public IExchangeFeed {
  public:
    BinanceFeed();
    void connect() override;
    void readLoop();

  private:
    net::io_context ioc;
    websocket::stream<tcp::socket> ws;
};