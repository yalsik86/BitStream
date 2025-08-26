#pragma once
#include "IExchangeFeed.hpp"

class CryptoComFeed : public IExchangeFeed {
  public:
    CryptoComFeed();
    void connect() override;
    void receiveUpdates() override;

  private:
    net::io_context ioc;
    ssl::context ssl_ctx;
    websocket::stream<ssl::stream<tcp::socket>> ws;
};