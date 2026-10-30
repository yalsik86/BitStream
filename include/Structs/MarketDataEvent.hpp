#pragma once
#include <array>
#include <cstdint>

struct MarketDataEvent {
    uint32_t sequence{};
    
    std::array<char, 16> symbol{};

    // cross-exchange spreads
    std::array<char, 16> exchange1{};
    std::array<char, 16> exchange2{};
    
    double spread12{};
    double spread21{};

    // global BBO
    double bestBidPrice{};
    double bestBidSize{};
    std::array<char, 16> bestBidExchange{};

    double bestAskPrice{};
    double bestAskSize{};
    std::array<char, 16> bestAskExchange{};

    // top-of-book imbalance
    double imbalance1{};
    double imbalance2{};
    // aggregate imbalance
    double aggImbalance{};

    // mid price
    // simple
    double simpleMid1{};
    double simpleMid2{};
    double simpleDivergence{};
    // weighted
    double weightedMid1{};
    double weightedMid2{};
    double weightedDivergence{};
};