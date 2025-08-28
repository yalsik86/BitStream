#pragma once
#include <string>

struct MarketDataEvent {
    std::string symbol;

    std::string exchange1;
    std::string exchange2;
    
    double spread12 = 0.0;
    double spread21 = 0.0;
};