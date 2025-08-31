#pragma once
#include "IExchangeFeed.hpp"
#include "../AggregatorEngine/AggregatorEngine.hpp"

class BinanceFeed : public IExchangeFeed {
  public:
    BinanceFeed(AggregatorEngine& engine);
    void connect() override;
    void receiveUpdates() override;
    std::optional<ExchangeUpdate> parseRaw(const std::string&) override;

  private:
    AggregatorEngine& engine;
    net::io_context ioc;
    ssl::context ssl_ctx;
    std::optional<websocket::stream<ssl::stream<tcp::socket>>> ws;
};