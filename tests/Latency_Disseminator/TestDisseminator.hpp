#pragma once
#include "DataDisseminator/DataDisseminator.hpp"
#include "DataDisseminator/Retransmitter.hpp"
#include "TestPublisher.hpp"
#include "TimedBuffer.hpp"
#include <chrono>

class TestDisseminator : public DataDisseminator {
  public:
    TestDisseminator(TestPublisher&, Retransmitter&);
    void start();
    void shutdown();
    void ingestEvent(const MarketDataEvent&);

  private:
    TestPublisher& publisher;
    Retransmitter& retransmitter;

    inline std::vector<uint8_t> serialize(const MarketDataEvent& event);
};