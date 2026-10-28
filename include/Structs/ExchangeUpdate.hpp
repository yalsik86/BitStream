#pragma once
#include <array>

#include "Enums/ExchangeID.hpp"

struct ExchangeUpdate {
    ExchangeId exchangeId{};
    bool hasReceivedUpdate{false};

    std::array<char, 8> symbol{};

    double bidPrice{};
    double bidSize {};
    double askPrice{};
    double askSize {};

    double lastPrice{};
};
