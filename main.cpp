#include "ExchangeConnectors/BinanceFeed.hpp"

int main() {
    BinanceFeed feed;

    feed.connect();
    feed.readLoop();

    return 0;
}