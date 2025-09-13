#pragma once
#include <string>

struct ExchangeUpdate {
    std::string exchange;
    std::string symbol;
    
    double bidPrice = 0.0;
    double bidSize  = 0.0;
    double askPrice = 0.0;
    double askSize  = 0.0;

    double lastPrice = 0.0;
};
