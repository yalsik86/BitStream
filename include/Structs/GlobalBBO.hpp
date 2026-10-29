#pragma once
#include <array>
#include <limits>

#include "Enums/ExchangeId.hpp"

struct GlobalBBO {
    double bestBid{-1.0};
    ExchangeId bestBidExchangeId{ExchangeId::Count};
    double bestBidSize{};

    double bestAsk{std::numeric_limits<double>::max()};
    ExchangeId bestAskExchangeId{ExchangeId::Count};
    double bestAskSize{};
};