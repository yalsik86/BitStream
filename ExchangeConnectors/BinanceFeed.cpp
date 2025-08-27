#include "BinanceFeed.hpp"

BinanceFeed::BinanceFeed(AggregatorEngine& engine) : 
    ssl_ctx(ssl::context::tls_client), ws(ioc, ssl_ctx), engine(engine) {
    ssl_ctx.set_default_verify_paths();
}

void BinanceFeed::connect() {
    boost::beast::flat_buffer buffer;
    tcp::resolver resolver(ioc);
    auto const results = resolver.resolve("stream.binance.com", "9443");

    net::connect(ws.next_layer().next_layer(), results);
    ws.next_layer().handshake(ssl::stream_base::client);
    ws.handshake("stream.binance.com", "/ws");
    std::cout <<"[+] Connected and handshake complete\n";

    nlohmann::json msg = {
        {"method", "SUBSCRIBE"},
        {"params", {"btcusdt@ticker"}},
        {"id", 1}
    };
    ws.write(net::buffer(msg.dump()));
}

void BinanceFeed::receiveUpdates() {
    beast::flat_buffer buffer;

    while(true) {
        try {
            ws.read(buffer);
            std::string msg = beast::buffers_to_string(buffer.data());
            engine.ingestRaw("Binance", msg);
            buffer.consume(buffer.size());
        } catch (const beast::system_error& e) {
            std::cerr <<"WebSocket read error: "<< e.what() << std::endl;
            break;
        }
    }
}