#include "CryptoComFeed.hpp"

CryptoComFeed::CryptoComFeed(AggregatorEngine& engine) : 
    engine(engine), ssl_ctx(ssl::context::tls_client) {
    ssl_ctx.set_default_verify_paths();
}

void CryptoComFeed::connect() {
    ws.emplace(ioc, ssl_ctx); // fresh websocket
    
    boost::beast::flat_buffer buffer;
    tcp::resolver resolver(ioc);
    auto const results = resolver.resolve("stream.crypto.com", "443");

    asio::connect(ws->next_layer().next_layer(), results);
    SSL_set_tlsext_host_name(ws->next_layer().native_handle(), "stream.crypto.com");
    ws->next_layer().handshake(ssl::stream_base::client);
    ws->handshake("stream.crypto.com", "/v2/market");
    spdlog::info("[Crypto.com] Connected and handshake complete");

    nlohmann::json msg = {
        {"method", "subscribe"},
        {"params", {
            {"channels", {"ticker.BTC_USDT"}}
        }}
    };
    ws->write(asio::buffer(msg.dump()));
    spdlog::info("[Crypto.com] Subscription message sent");
}

void CryptoComFeed::receiveUpdates(std::stop_token stoken) {
    beast::flat_buffer buffer;

    while(!stoken.stop_requested()) {
        try {
            ws->read(buffer);
            std::string raw = beast::buffers_to_string(buffer.data());
            
            auto update = parseRaw(raw);
            if(update.has_value()) {
                engine.ingestUpdate(update.value());
            }
            buffer.consume(buffer.size());
        } catch (const beast::system_error& e) {
            spdlog::warn("[Crypto.com] WebSocket read error: {}", e.code().message());
            break;
        }
    }
}

void CryptoComFeed::run(std::stop_token stoken) {
    while(!stoken.stop_requested()) {
        try {
            connect();
            receiveUpdates(stoken);
        } catch (const std::exception &e) {
            if(stoken.stop_requested()) break;
            spdlog::error("[Crypto.com] Fatal error: {} - retrying in 2s...", e.what());
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
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

void CryptoComFeed::disconnect() {
    if(ws && ws->is_open()) {
        beast::error_code ec;
        ws->close(websocket::close_code::normal, ec);
        if(ec) {
            spdlog::warn("[Crypto.com] Disconnect error: {}", ec.message());
        } else {
            spdlog::info("[Crypto.com] Disconnected cleanly");
        }
    }
    ws.reset();
}