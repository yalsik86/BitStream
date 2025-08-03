#include "CryptoComFeed.hpp"

CryptoComFeed::CryptoComFeed() : ssl_ctx(ssl::context::tls_client), ws(ioc, ssl_ctx) {
    ssl_ctx.set_default_verify_paths();
}

void CryptoComFeed::connect() {
    boost::beast::flat_buffer buffer;
    tcp::resolver resolver(ioc);
    auto const results = resolver.resolve("stream.crypto.com", "443");

    net::connect(ws.next_layer().next_layer(), results);
    SSL_set_tlsext_host_name(ws.next_layer().native_handle(), "stream.crypto.com");
    ws.next_layer().handshake(ssl::stream_base::client);
    ws.handshake("stream.crypto.com", "/v2/market");
    std::cout <<"[+] Connected and handshake complete\n";

    nlohmann::json msg = {
        {"method", "subscribe"},
        {"params", {
            {"channels", {"ticker.BTC_USDT"}}
        }}
    };
    ws.write(net::buffer(msg.dump()));
}

void CryptoComFeed::readLoop() {
    beast::flat_buffer buffer;

    while(true) {
        try {
            ws.read(buffer);
            std::string msg = beast::buffers_to_string(buffer.data());
            std::cout <<"[Crypto.com] Received: "<< msg << std::endl;
            buffer.consume(buffer.size());
        } catch (const beast::system_error& e) {
            std::cerr <<"WebSocket read error: "<< e.what() << std::endl;
            break;
        }
    }
}
