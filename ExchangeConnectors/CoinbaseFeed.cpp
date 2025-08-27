#include "CoinbaseFeed.hpp"

CoinbaseFeed::CoinbaseFeed(AggregatorEngine& engine) : 
    ssl_ctx(ssl::context::tls_client), ws(ioc, ssl_ctx), engine(engine) {
    ssl_ctx.set_default_verify_paths();
}

void CoinbaseFeed::connect() {
    boost::beast::flat_buffer buffer;
    tcp::resolver resolver(ioc);
    auto const results = resolver.resolve("ws-feed.exchange.coinbase.com", "443");

    net::connect(ws.next_layer().next_layer(), results);
    SSL_set_tlsext_host_name(ws.next_layer().native_handle(), "ws-feed.exchange.coinbase.com");
    ws.next_layer().handshake(ssl::stream_base::client);
    ws.handshake("ws-feed.exchange.coinbase.com", "/");
    std::cout <<"[+][Coinbase] Connected and handshake complete\n";

    nlohmann::json msg = {
        {"type", "subscribe"},
        {"product_ids", {"BTC-USD"}},
        {"channels", {"ticker", "heartbeat"}} 
    };
    ws.write(net::buffer(msg.dump()));
}

void CoinbaseFeed::receiveUpdates() {
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


std::optional<ExchangeUpdate> CoinbaseFeed::parseRaw(const std::string& raw) {
    auto j = nlohmann::json::parse(raw, nullptr, false);
    if(j.is_discarded()) {
        return std::nullopt;
    }

    if(!j.contains("type") || j["type"] != "ticker") {
        return std::nullopt;
    }

    ExchangeUpdate update;
    update.exchange = "Coinbase";
    update.symbol = j["product_id"];

    update.bidPrice = std::stod(j["best_bid"].get<std::string>());
    update.bidSize = std::stod(j["best_bid_size"].get<std::string>());
    update.askPrice = std::stod(j["best_ask"].get<std::string>());
    update.askSize = std::stod(j["best_ask_size"].get<std::string>());
    update.lastPrice = std::stod(j["price"].get<std::string>());

    return update;
}