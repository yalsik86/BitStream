#pragma once
#include "IExchangeFeed.hpp"
#include <nlohmann/json.hpp>

class CryptoComFeed : public IExchangeFeed {
  public:
    CryptoComFeed();
    void connect() override;
    void readLoop();

  private:
    net::io_context ioc;
    ssl::context ssl_ctx;
    websocket::stream<ssl::stream<tcp::socket>> ws;
};