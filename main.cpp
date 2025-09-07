#include "ExchangeConnectors/BinanceFeed.hpp"
#include "ExchangeConnectors/CoinbaseFeed.hpp"
#include "ExchangeConnectors/KrakenFeed.hpp"
#include "ExchangeConnectors/CryptoComFeed.hpp"
#include "ExchangeConnectors/CryptoComFeed.hpp"
#include "AggregatorEngine/AggregatorEngine.hpp"
#include "MarketDataRouter/MarketDataRouter.hpp"

int main() {
    MarketDataRouter router;
    AggregatorEngine engine(router);

    engine.addConnector(std::make_unique<BinanceFeed>(engine));
    engine.addConnector(std::make_unique<CoinbaseFeed>(engine));
    engine.addConnector(std::make_unique<KrakenFeed>(engine));
    engine.addConnector(std::make_unique<CryptoComFeed>(engine));

    engine.start();

    return 0;
}