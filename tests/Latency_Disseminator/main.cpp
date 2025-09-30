#include "DataDisseminator/Retransmitter.hpp"
#include "TestPublisher.hpp"
#include "TestDisseminator.hpp"
#include "Structs/MarketDataEvent.hpp"
#include "Protocol/market_data.pb.h"
#include <random>

void createTestEvent(MarketDataEvent& event);

int main() {
    // ------ Timers and distibution setup ------
    std::random_device rd;
    std::mt19937 gen(rd());

    // Burst size: 20–200 msgs
    std::uniform_int_distribution<> burst(20, 200);
    // Delay per message: 5–40 ms
    std::uniform_int_distribution<> delay(5000, 40000);

    int total_msgs = 10000;

    // ------ Sample Event Setup ------
    MarketDataEvent event;
    createTestEvent(event);

    // ------ Disseminator test ------
    TestPublisher publisher;
    Retransmitter retransmitter;
    TestDisseminator disseminator(publisher, retransmitter);
    disseminator.start();

    auto next_send = std::chrono::high_resolution_clock::now();

    for(int i=0; i<total_msgs;) {
        int burst_size = burst(gen);
        auto interval = std::chrono::microseconds(delay(gen));

        for(int j=0; j<burst_size && i<total_msgs; j++, i++) {
            next_send += interval;
            while(next_send > std::chrono::high_resolution_clock::now()) {
                // spin-wait
            }

            disseminator.ingestEvent(event);
        }
    }

    disseminator.shutdown();
    publisher.reportStats();
    // ------ ------ ------ ------
}


void createTestEvent(MarketDataEvent& e) {
    // sequence number
    e.sequence = 200;

    // symbol
    std::copy_n("BTCUSDT", 7, e.symbol.begin());  

    // exchange names
    std::copy_n("Coinbase", 8, e.exchange1.begin());
    std::copy_n("Binance", 7, e.exchange2.begin());

    // spreads
    e.spread12 = -0.8300;
    e.spread21 = 0.8500;

    // global BBO
    e.bestBidPrice = 115860.8400;
    e.bestBidSize  = 1.2756;
    std::copy_n("Kraken", 6, e.bestBidExchange.begin());

    e.bestAskPrice = 115802.1600;
    e.bestAskSize  = 3.0531;
    std::copy_n("Binance", 7, e.bestAskExchange.begin());

    // imbalance
    e.imbalance1 = 0.7512;
    e.imbalance2 = 0.7266;
    e.aggImbalance = 0.7932;

    // mid prices
    e.simpleMid1 = 115802.9950;
    e.simpleMid2 = 115802.1550;
    e.simpleDivergence = 0.8400;

    e.weightedMid1 = 115802.9988;
    e.weightedMid2 = 115802.1586;
    e.weightedDivergence = 0.8401;
}