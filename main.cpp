#include "ExchangeConnectors/BinanceFeed.hpp"
#include "ExchangeConnectors/CoinbaseFeed.hpp"
#include "ExchangeConnectors/KrakenFeed.hpp"
#include "ExchangeConnectors/CryptoComFeed.hpp"

int main() {
    // CoinbaseFeed feed;
    BinanceFeed feed;
    // KrakenFeed feed;
    // CryptoComFeed feed;

    feed.connect();
    feed.readLoop();

    return 0;
}