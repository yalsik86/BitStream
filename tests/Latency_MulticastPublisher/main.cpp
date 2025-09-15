#include "DataDisseminator/DataDisseminator.hpp"
#include "TestPublisher_lf.hpp"
#include "TestPublisher.hpp"
#include "Structs/MarketDataEvent.hpp"
#include "Protocol/market_data.pb.h"
#include <random>

void createTestEvent(MarketDataEvent& event);
std::vector<uint8_t> serializeTestEvent(const MarketDataEvent& event);

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
    auto buffer = serializeTestEvent(event);

    // ------ lock-based test ------
    TestPublisher publisher1;
    DataDisseminator disseminator1(publisher1);
    disseminator1.start();
    publisher1.start();

    auto next_send = std::chrono::high_resolution_clock::now();

    for(int i=0; i<total_msgs;) {
        int burst_size = burst(gen);
        auto interval = std::chrono::microseconds(delay(gen));

        for(int j=0; j<burst_size && i<total_msgs; j++, i++) {
            auto copy = buffer; // fresh copies to avoid invalidation upon std::move into TimedBuffer
            
            next_send += interval;
            while(next_send > std::chrono::high_resolution_clock::now()) {
                // spin-wait
            }

            publisher1.push(copy);
        }
    }

    disseminator1.shutdown();
    publisher1.stop();
    publisher1.reportStats();
    // ------ ------ ------ ------

    // ------ lockfree test ------
    TestPublisher_lf publisher2;
    DataDisseminator disseminator2(publisher2);
    disseminator2.start();
    publisher2.start();

    next_send = std::chrono::high_resolution_clock::now();

    for(int i=0; i<total_msgs;) {
        int burst_size = burst(gen);
        auto interval = std::chrono::microseconds(delay(gen));

        for(int j=0; j<burst_size && i<total_msgs; j++, i++) {
            auto copy = buffer; // fresh copies to avoid invalidation upon std::move into TimedBuffer
            
            next_send += interval;
            while(next_send > std::chrono::high_resolution_clock::now()) {
                // spin-wait
            }

            publisher2.push(copy);
        }
    }

    disseminator2.shutdown();
    publisher2.stop();
    publisher2.reportStats();
    // ------ ------ ------ ------
}


void createTestEvent(MarketDataEvent& e) {
    // sequence number
    e.sequence = 200;

    // symbol
    std::copy_n("BTCUSDT", 7, e.symbol.begin());  

    // exchange names
    std::copy_n("Crypto.com", 11, e.exchange1.begin());
    std::copy_n("Binance", 7, e.exchange2.begin());

    // spreads
    e.spread12 = -0.8300;
    e.spread21 = 0.8500;

    // global BBO
    e.bestBidPrice = 115860.8400;
    e.bestBidSize  = 1.2756;
    std::copy_n("Coinbase", 8, e.bestBidExchange.begin());

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

std::vector<uint8_t> serializeTestEvent(const MarketDataEvent& event) {
    MarketDataEventProto msg;

    msg.set_sequence(event.sequence);
    msg.set_symbol(event.symbol.data(), event.symbol.size());
    msg.set_exchange1(event.exchange1.data(), event.exchange1.size());
    msg.set_exchange2(event.exchange2.data(), event.exchange2.size());

    msg.set_spread12(event.spread12);
    msg.set_spread21(event.spread21);
    msg.set_bestbidprice(event.bestBidPrice);
    msg.set_bestbidsize(event.bestBidSize);
    msg.set_bestbidexchange(event.bestBidExchange.data(), event.bestBidExchange.size());

    msg.set_bestaskprice(event.bestAskPrice);
    msg.set_bestasksize(event.bestAskSize);
    msg.set_bestaskexchange(event.bestAskExchange.data(), event.bestAskExchange.size());

    msg.set_imbalance1(event.imbalance1);
    msg.set_imbalance2(event.imbalance2);
    msg.set_aggimbalance(event.aggImbalance);

    msg.set_simplemid1(event.simpleMid1);
    msg.set_simplemid2(event.simpleMid2);
    msg.set_simpledivergence(event.simpleDivergence);

    msg.set_weightedmid1(event.weightedMid1);
    msg.set_weightedmid2(event.weightedMid2);
    msg.set_weighteddivergence(event.weightedDivergence);

    size_t size = msg.ByteSizeLong();
    std::vector<uint8_t> buffer(size);

    if(!msg.SerializeToArray(buffer.data(), buffer.size())) {
        throw std::runtime_error("Serialization failed");
    }
    return buffer;
}