#pragma once
#include <string>

struct MarketDataEvent {
    std::string symbol;

    // cross-exchange spreads
    std::string exchange1;
    std::string exchange2;
    
    double spread12 = 0.0;
    double spread21 = 0.0;

    // global BBO
    double bestBidPrice = 0.0;
    double bestBidSize  = 0.0;
    std::string bestBidExchange;

    double bestAskPrice = 0.0;
    double bestAskSize  = 0.0;
    std::string bestAskExchange;
};