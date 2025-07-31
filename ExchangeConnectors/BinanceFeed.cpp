#include "BinanceFeed.hpp"
#include <iostream>

BinanceFeed::BinanceFeed() : ws(ioc) {}

void BinanceFeed::connect() {
    tcp::resolver resolver(ioc);
    auto endpoints = resolver.resolve("stream.binance.com", "9443");
    net::connect(ws.next_layer(), endpoints);
    ws.handshake("stream.binance.com", "/ws/btcusdt@depth");
}

void BinanceFeed::readLoop() {
    beast::flat_buffer buffer;

    while(true) {
        try {
            ws.read(buffer);
            std::string msg = beast::buffers_to_string(buffer.data());
            std::cout <<"[Binance] Received: "<< msg << std::endl;
            buffer.consume(buffer.size());
        } catch (const beast::system_error& e) {
            std::cerr <<"WebSocket read error: "<< e.what() << std::endl;
            break;
        }
    }
}