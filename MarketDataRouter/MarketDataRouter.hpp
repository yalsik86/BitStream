#pragma once
#include "../Protocol/market_data.pb.h"
#include "../Structs/MarketDataEvent.hpp"
#include <vector>
#include <string>

class MarketDataRouter {
  public:
    void ingestEvent(const MarketDataEvent& event);
  
  private:
    // ------ Serialization ------
    inline std::vector<uint8_t> serialize(const MarketDataEvent& event);

    inline void coutEncoded(const std::vector<uint8_t>& buffer);
};