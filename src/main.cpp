#include "FeedHandlers/BinanceFeed.hpp"
#include "FeedHandlers/CoinbaseFeed.hpp"
#include "FeedHandlers/KrakenFeed.hpp"
#include "FeedHandlers/CryptoComFeed.hpp"
#include "FeedHandlers/CryptoComFeed.hpp"
#include "AggregatorEngine/AggregatorEngine.hpp"
#include "DataDisseminator/DataDisseminator.hpp"
#include "DataDisseminator/MulticastPublisher.hpp"

int main() {
    MulticastPublisher publisher1;
    DataDisseminator router(publisher1);
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