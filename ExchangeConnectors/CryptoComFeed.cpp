#include "CryptoComFeed.hpp"

CryptoComFeed::CryptoComFeed(AggregatorEngine& engine) : 
    ssl_ctx(ssl::context::tls_client), ws(ioc, ssl_ctx), engine(engine) {
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
    std::cout <<"[+][Crypto.com] Connected and handshake complete\n";

    nlohmann::json msg = {
        {"method", "subscribe"},
        {"params", {
            {"channels", {"ticker.BTC_USDT"}}
        }}
    };
    ws.write(net::buffer(msg.dump()));
}

void CryptoComFeed::receiveUpdates() {
    beast::flat_buffer buffer;

    while(true) {
        try {
            ws.read(buffer);
            std::string raw = beast::buffers_to_string(buffer.data());
            
            auto update = parseRaw(raw);
            if(update.has_value()) {
                engine.ingestUpdate(update.value());
            }
            buffer.consume(buffer.size());
        } catch (const beast::system_error& e) {
            std::cerr <<"WebSocket read error: "<< e.what() << std::endl;
            break;
        }
    }
}

std::optional<ExchangeUpdate> CryptoComFeed::parseRaw(const std::string& raw) {
    auto j = nlohmann::json::parse(raw, nullptr, false);
    if(j.is_discarded()) {
        return std::nullopt;
    }

    if (!j.contains("result") || !j["result"].contains("channel")) {
        return std::nullopt;
    }

    if(j["result"]["channel"] != "ticker") {
        return std::nullopt;
    }

    if(!j["result"].contains("data") || !j["result"]["data"].is_array() || j["result"]["data"].empty()) {
        return std::nullopt;
    }

    const auto& data = j["result"]["data"][0];

    ExchangeUpdate update;
    update.exchange = "Crypto.com";
    update.symbol = data["i"].get<std::string>();

    update.bidPrice = std::stod(data["b"].get<std::string>());
    update.bidSize = std::stod(data["bs"].get<std::string>());
    update.askPrice = std::stod(data["k"].get<std::string>());
    update.askSize = std::stod(data["ks"].get<std::string>());

    return update;
}