#include "ExchangeConnectors/BinanceFeed.hpp"
#include "ExchangeConnectors/CoinbaseFeed.hpp"

int main() {
    CoinbaseFeed feed;
    // BinanceFeed feed;

    feed.connect();
    feed.readLoop();

    return 0;
}