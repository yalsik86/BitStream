#include "KrakenFeed.hpp"

KrakenFeed::KrakenFeed(AggregatorEngine& engine) : 
    ssl_ctx(ssl::context::tls_client), engine(engine) {
    ssl_ctx.set_default_verify_paths();
}

void KrakenFeed::connect() {
    ws.emplace(ioc, ssl_ctx);

    boost::beast::flat_buffer buffer;
    tcp::resolver resolver(ioc);
    auto const results = resolver.resolve("ws.kraken.com", "443");

    net::connect(ws->next_layer().next_layer(), results);
    SSL_set_tlsext_host_name(ws->next_layer().native_handle(), "ws.kraken.com");
    ws->next_layer().handshake(ssl::stream_base::client);
    ws->handshake("ws.kraken.com", "/v2");
    std::cout <<"[+][Kraken] Connected and handshake complete\n";

    nlohmann::json msg = {
        {"method", "subscribe"},
        {"params", {
            {"channel", "ticker"},
            {"symbol", {"BTC/USD"}}
        }}
    };
    ws->write(net::buffer(msg.dump()));
}

void KrakenFeed::receiveUpdates() {
    beast::flat_buffer buffer;

    while(true) {
        try {
            ws->read(buffer);
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

std::optional<ExchangeUpdate> KrakenFeed::parseRaw(const std::string& raw) {
    auto j = nlohmann::json::parse(raw, nullptr, false);
    if(j.is_discarded()) {
        return std::nullopt;
    }

    if(!j.contains("channel") || j["channel"] != "ticker") {
        return std::nullopt;
    }

    if(!j.contains("data") || !j["data"].is_array() || j["data"].empty()) {
        return std::nullopt;
    }

    const auto& data = j["data"][0];

    ExchangeUpdate update;
    update.exchange = "Kraken";
    update.symbol = data["symbol"];

    update.bidPrice = data["bid"].get<double>();
    update.bidSize = data["bid_qty"].get<double>();
    update.askPrice = data["ask"].get<double>();
    update.askSize = data["ask_qty"].get<double>();
    update.lastPrice = data["last"].get<double>();

    return update;
}