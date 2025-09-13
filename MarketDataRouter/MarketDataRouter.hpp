#pragma once
#include "../Protocol/market_data.pb.h"
#include "../Structs/MarketDataEvent.hpp"
#include "MulticastPublisher.hpp"
#include <vector>
#include <string>

class MarketDataRouter {
  public:
    MarketDataRouter(MulticastPublisher&);
    void start();
    void shutdown();
    void ingestEvent(const MarketDataEvent& event);
  
  private:
    MulticastPublisher& multicastPublisher;
    // ------ Serialization ------
    inline std::vector<uint8_t> serialize(const MarketDataEvent& event);

    inline void coutEncoded(const std::vector<uint8_t>& buffer);
};