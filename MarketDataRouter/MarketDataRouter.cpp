#include "MarketDataRouter.hpp"

MarketDataRouter::MarketDataRouter(MulticastPublisher& publisher1) : multicastPublisher(publisher1) {}

void MarketDataRouter::ingestEvent(const MarketDataEvent& event) {
    auto buffer = serialize(event);

    multicastPublisher.publish(buffer);
    // coutEncoded(buffer);
}

inline std::vector<uint8_t> MarketDataRouter::serialize(const MarketDataEvent& event) {
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

inline void MarketDataRouter::coutEncoded(const std::vector<uint8_t>& buffer) {
    std::cout << std::dec << "------------------\n";
    std::cout << std::dec << "Buffer size: " << buffer.size() << " bytes\n";
    for(auto &b : buffer) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
        << static_cast<int>(b) << " ";
    }
    std::cout << std::dec << "\n------------------\n";
}