#pragma once
#include "IExchangeFeed.hpp"
#include "../AggregatorEngine/AggregatorEngine.hpp"

class KrakenFeed : public IExchangeFeed {
  public:
    KrakenFeed(AggregatorEngine& engine);
    void run(std::stop_token) override;
    void connect() override;
    void receiveUpdates(std::stop_token) override;
    std::optional<ExchangeUpdate> parseRaw(const std::string&) override;
    void disconnect() override;

  private:
    AggregatorEngine& engine;
    asio::io_context ioc;
    ssl::context ssl_ctx;
    std::optional<websocket::stream<ssl::stream<tcp::socket>>> ws;
};