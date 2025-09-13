#pragma once
#include "IExchangeFeed.hpp"
#include "AggregatorEngine/AggregatorEngine.hpp"

class CryptoComFeed : public IExchangeFeed {
  public:
    CryptoComFeed(AggregatorEngine& engine);
    void run(std::atomic<bool>&) override;
    void connect() override;
    void receiveUpdates(std::atomic<bool>&) override;
    std::optional<ExchangeUpdate> parseRaw(const std::string&) override;
    void disconnect() override;

  private:
    AggregatorEngine& engine;
    asio::io_context ioc;
    ssl::context ssl_ctx;
    std::optional<websocket::stream<ssl::stream<tcp::socket>>> ws;
};