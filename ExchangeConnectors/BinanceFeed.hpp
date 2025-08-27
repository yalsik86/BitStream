#pragma once
#include "IExchangeFeed.hpp"
#include "../AggregatorEngine/AggregatorEngine.hpp"

class BinanceFeed : public IExchangeFeed {
  public:
    BinanceFeed(AggregatorEngine& engine);
    void connect() override;
    void receiveUpdates() override;

  private:
    AggregatorEngine& engine;
    net::io_context ioc;
    ssl::context ssl_ctx;
    websocket::stream<ssl::stream<tcp::socket>> ws;
};