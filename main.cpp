#include "ExchangeConnectors/BinanceFeed.hpp"
#include "ExchangeConnectors/CoinbaseFeed.hpp"
#include "ExchangeConnectors/KrakenFeed.hpp"
#include "ExchangeConnectors/CryptoComFeed.hpp"
#include "ExchangeConnectors/CryptoComFeed.hpp"
#include "AggregatorEngine/AggregatorEngine.hpp"

int main() {
    // CoinbaseFeed feed;
    // BinanceFeed feed;
    // KrakenFeed feed;
    // CryptoComFeed feed;

    // feed.connect();
    // feed.readLoop();

    AggregatorEngine engine;

    engine.addConnector(std::make_unique<BinanceFeed>(engine));
    engine.addConnector(std::make_unique<CoinbaseFeed>(engine));

    engine.start();

    return 0;
}