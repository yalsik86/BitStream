#pragma once
#include <array>
#include <limits>

#include "Enums/ExchangeID.hpp"

struct GlobalBBO {
    double bestBid{-1.0};
    ExchangeId bestBidExchangeId{};
    double bestBidSize{};

    double bestAsk{std::numeric_limits<double>::max()};
    ExchangeId bestAskExchangeId{};
    double bestAskSize{};
};