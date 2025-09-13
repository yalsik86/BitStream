#include "ExchangeConnectors/BinanceFeed.hpp"
#include "ExchangeConnectors/CoinbaseFeed.hpp"
#include "ExchangeConnectors/KrakenFeed.hpp"
#include "ExchangeConnectors/CryptoComFeed.hpp"
#include "ExchangeConnectors/CryptoComFeed.hpp"
#include "AggregatorEngine/AggregatorEngine.hpp"
#include "MarketDataRouter/MarketDataRouter.hpp"
#include "MarketDataRouter/MulticastPublisher.hpp"

int main() {
    MulticastPublisher publisher1;
    MarketDataRouter router(publisher1);
    AggregatorEngine engine(router);

    engine.addConnector(std::make_unique<BinanceFeed>(engine));
    engine.addConnector(std::make_unique<CoinbaseFeed>(engine));
    engine.addConnector(std::make_unique<KrakenFeed>(engine));
    engine.addConnector(std::make_unique<CryptoComFeed>(engine));

    engine.start();

    std::this_thread::sleep_for(std::chrono::seconds(20));

    engine.shutdown();
    return 0;
}