#pragma once
#include <string>
#include <limits>

struct GlobalBBO {
    double bestBid = -1.0;
    std::string bestBidEx;
    double bestBidSize = 0.0;

    double bestAsk = std::numeric_limits<double>::max();
    std::string bestAskEx;
    double bestAskSize = 0.0;
};