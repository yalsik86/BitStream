#include "FeedHandlers/KrakenFeed.hpp"

KrakenFeed::KrakenFeed(AggregatorEngine& engine) : 
    engine(engine), ssl_ctx(ssl::context::tls_client) {
    ssl_ctx.set_default_verify_paths();
}

void KrakenFeed::connect() {
    StageTimer connect_timer("[Kraken] Feed Handler connect()");
    {
        StageTimer timer("[Kraken] WebSocket Construction");
        ws.emplace(ioc, ssl_ctx);
    }

    tcp::resolver resolver(ioc);
    tcp::resolver::results_type results;
    {
        StageTimer timer("[Kraken] DNS Resolution");
        results = resolver.resolve(tcp::v4(), "ws.kraken.com", "443");
    }

    {
        StageTimer timer("[Kraken] TCP Connection");
        asio::connect(ws->next_layer().next_layer(), results);
    }

    SSL_set_tlsext_host_name(ws->next_layer().native_handle(), "ws.kraken.com");
    {
        StageTimer timer("[Kraken] SSL Handshake");
        ws->next_layer().handshake(ssl::stream_base::client);
    }
    {
        StageTimer timer("[Kraken] WebSocket Upgrade");
        ws->handshake("ws.kraken.com", "/v2");
    }
    spdlog::info("[Kraken] Connected and handshake complete");

    nlohmann::json msg = {
        {"method", "subscribe"},
        {"params", {
            {"channel", "ticker"},
            {"symbol", {"BTC/USDT"}}
        }}
    };

    auto payload = msg.dump();
    {
        StageTimer timer("[Kraken] Subscription Write");
        ws->write(asio::buffer(payload));
    }
    spdlog::info("[Kraken] Subscription message sent");
}

void KrakenFeed::receiveUpdates(std::atomic<bool>& run_flag) {
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
            spdlog::warn("[Kraken] WebSocket read error: {}", e.code().message());
            break;
        }
    }
}

void KrakenFeed::run(std::atomic<bool>& run_flag) {
    while(run_flag) {
        try {
            connect();
            receiveUpdates(run_flag);
        } catch (const std::exception &e) {
            spdlog::error("[Kraken] Fatal error: {} - retrying in 2s...", e.what());
        }
        if(!run_flag) break;
        std::this_thread::sleep_for(std::chrono::seconds(2));
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

    const auto& data = j["data"][0]; // TODO: check if get_ref is needed for performance

    ExchangeUpdate update;
    update.exchangeId = ExchangeId::Kraken;

    const auto& symbol = data["symbol"].get_ref<const std::string&>();
    StringUtils::assignStringToArray(update.symbol, symbol);

    update.bidPrice = data["bid"].get<double>();
    update.bidSize = data["bid_qty"].get<double>();
    update.askPrice = data["ask"].get<double>();
    update.askSize = data["ask_qty"].get<double>();
    update.lastPrice = data["last"].get<double>();

    return update;
}

void KrakenFeed::disconnect() {
    if(ws && ws->is_open()) {
        beast::error_code ec;
        ws->close(websocket::close_code::normal, ec);
        if(ec) {
            spdlog::warn("[Kraken] Disconnect error: {}", ec.message());
        } else {
            spdlog::info("[Kraken] Disconnected gracefully");
        }
    }
    ws.reset();
}