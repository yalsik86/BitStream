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
    std::cout <<"[+][Binance] Connected and handshake complete\n";

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

std::optional<ExchangeUpdate> BinanceFeed::parseRaw(const std::string& raw) {
    auto j = nlohmann::json::parse(raw, nullptr, false);
    if(j.is_discarded()) {
        return std::nullopt;
    }

    if(!j.contains("e") || j["e"] != "24hrTicker") {
        return std::nullopt;
    }

    ExchangeUpdate update;
    update.exchange = "Binance";
    update.symbol = j["s"];

    update.bidPrice = std::stod(j["b"].get<std::string>());
    update.bidSize = std::stod(j["B"].get<std::string>());
    update.askPrice = std::stod(j["a"].get<std::string>());
    update.askSize = std::stod(j["A"].get<std::string>());
    update.lastPrice = std::stod(j["c"].get<std::string>());

    return update;
}
