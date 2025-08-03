#include "ExchangeConnectors/BinanceFeed.hpp"
#include "ExchangeConnectors/CoinbaseFeed.hpp"
#include "ExchangeConnectors/KrakenFeed.hpp"

int main() {
    // CoinbaseFeed feed;
    // BinanceFeed feed;
    KrakenFeed feed;

    feed.connect();
    feed.readLoop();

    return 0;
}