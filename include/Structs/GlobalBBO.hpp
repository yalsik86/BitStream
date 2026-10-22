#pragma once
#include <array>
#include <limits>

struct GlobalBBO {
    double bestBid{-1.0};
    std::array<char, 16> bestBidEx{};
    double bestBidSize{};

    double bestAsk{std::numeric_limits<double>::max()};
    std::array<char, 16> bestAskEx{};
    double bestAskSize{};
};