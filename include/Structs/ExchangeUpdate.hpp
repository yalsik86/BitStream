#pragma once
#include <array>

#include "Enums/ExchangeId.hpp"

struct ExchangeUpdate {
    ExchangeId exchangeId{};
    bool hasReceivedUpdate{false};

    std::array<char, 16> symbol{};

    double bidPrice{};
    double bidSize {};
    double askPrice{};
    double askSize {};

    double lastPrice{};
};

/* 
hasRecievedUpdate field in ExchangeUpdate struct:
there can be certain fields that need to check old snapshots and clear stale data before processing new updates 
and if the data arrives for the first time (ie. it's the first snapshot) 
then we don't run the stale data clearing logic 

it is left untouched in FeedHandlers
all first time updates will observe hasRecievedUpdate = false
all subsequent updates will observe hasRecievedUpdate = true (ensured in AggregatorEngine::ingestUpdate)
*/