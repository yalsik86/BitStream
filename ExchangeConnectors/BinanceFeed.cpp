#include "BinanceFeed.hpp"

BinanceFeed::BinanceFeed(AggregatorEngine& engine) : 
    engine(engine), ssl_ctx(ssl::context::tls_client) {
    ssl_ctx.set_default_verify_paths();
}

void BinanceFeed::connect() {
    ws.emplace(ioc, ssl_ctx);

    tcp::resolver resolver(ioc);
    auto const results = resolver.resolve("stream.binance.com", "9443");

    asio::connect(ws->next_layer().next_layer(), results);
    SSL_set_tlsext_host_name(ws->next_layer().native_handle(), "stream.binance.com");
    ws->next_layer().handshake(ssl::stream_base::client);
    ws->handshake("stream.binance.com", "/ws");
    spdlog::info("[Binance] Connected and handshake complete");

    nlohmann::json msg = {
        {"method", "SUBSCRIBE"},
        {"params", {"btcusdt@ticker"}},
        {"id", 1}
    };
    ws->write(asio::buffer(msg.dump()));
    spdlog::info("[Binance] Subscription message sent");
}

void BinanceFeed::receiveUpdates(std::atomic<bool>& run_flag) {
    beast::flat_buffer buffer;

    while(run_flag) {
        try {
            ws->read(buffer);
            std::string raw = beast::buffers_to_string(buffer.data());

            auto update = parseRaw(raw);
            if(update.has_value()) {
                engine.ingestUpdate(update.value());
            }
            buffer.consume(buffer.size());
        } catch (const beast::system_error& e) {
            spdlog::warn("[Binance] WebSocket read error: {}", e.code().message());
            break;
        }
    }
}

void BinanceFeed::run(std::atomic<bool>& run_flag) {
    while(run_flag) {
        try {
            connect();
            receiveUpdates(run_flag);
        } catch (const std::exception &e) {
            spdlog::error("[Binance] Fatal error: {} - retrying in 2s...", e.what());
        }
        if(!run_flag) break;
        std::this_thread::sleep_for(std::chrono::seconds(2));
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

void BinanceFeed::disconnect() {
    if(ws && ws->is_open()) {
        beast::error_code ec;
        ws->close(websocket::close_code::normal, ec);
        if(ec) {
            spdlog::warn("[Binance] Disconnect error: {}", ec.message());
        } else {
            spdlog::info("[Binance] Disconnected gracefully");
        }
    }
    ws.reset();
}