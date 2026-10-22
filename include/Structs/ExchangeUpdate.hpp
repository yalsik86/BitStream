#pragma once
#include <array>

struct ExchangeUpdate {
    std::array<char, 16> exchange{};
    std::array<char, 8> symbol{};

    double bidPrice{};
    double bidSize {};
    double askPrice{};
    double askSize {};

    double lastPrice{};
};
