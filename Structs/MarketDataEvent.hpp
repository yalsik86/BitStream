#pragma once
#include <array>
#include <cstdint>

struct MarketDataEvent {
    uint32_t sequence = 0;
    
    std::array<char, 16> symbol;

    // cross-exchange spreads
    std::array<char, 16> exchange1;
    std::array<char, 16> exchange2;
    
    double spread12 = 0.0;
    double spread21 = 0.0;

    // global BBO
    double bestBidPrice = 0.0;
    double bestBidSize  = 0.0;
    std::array<char, 16> bestBidExchange;

    double bestAskPrice = 0.0;
    double bestAskSize  = 0.0;
    std::array<char, 16> bestAskExchange;

    // top-of-book imbalance
    double imbalance1 = 0.0;
    double imbalance2 = 0.0;
    // aggregate imbalance
    double aggImbalance = 0.0;

    // mid price
    // simple
    double simpleMid1 = 0.0;
    double simpleMid2 = 0.0;
    double simpleDivergence = 0.0;
    // weighted
    double weightedMid1 = 0.0;
    double weightedMid2 = 0.0;
    double weightedDivergence = 0.0;
};