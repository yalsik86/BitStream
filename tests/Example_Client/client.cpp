#include <boost/asio.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include "../../src/Protocol/market_data.pb.h" 
#include "../../include/Structs/MarketDataEvent.hpp"

namespace net = boost::asio;
using boost::asio::ip::udp;

inline void coutEvent(const MarketDataEvent& event) {
    std::cout << std::fixed << std::setprecision(4)
    <<"------["<<event.symbol.data()<<"]------\n"
    <<"Sequence Number: "<<event.sequence<<"\n"
    <<"["<<event.exchange1.data()<<"] - ["<<event.exchange2.data()<<"]\n"
    <<"| Spread12: "<<event.spread12<<" | Spread21: "<<event.spread21<<"\n"
    <<"| Global BBO: \n"
    <<"    - Best Bid: "<<event.bestBidPrice<<" - Size: "<<event.bestBidSize<<" @ "<<event.bestBidExchange.data()<<"\n"
    <<"    - Best Ask: "<<event.bestAskPrice<<" - Size: "<<event.bestAskSize<<" @ "<<event.bestAskExchange.data()<<"\n"
    <<"| Imbalance1: "<<event.imbalance1<<" | Imbalance2: "<<event.imbalance2<<"\n"
    <<"| Aggregate Imbalance: "<<event.aggImbalance<<"\n"
    <<"| Mid-Prices:\n"
    <<"| Simple_Mid1: "<<event.simpleMid1<<" | Simple_Mid2: "<<event.simpleMid2<<"\n"
    <<"| Weighted_Mid1: "<<event.weightedMid1<<" | Weighted_Mid2: "<<event.weightedMid2<<"\n"
    <<"    - Simple Divergence: "<<event.simpleDivergence<<"\n"
    <<"    - Weighted Divergence: "<<event.weightedDivergence<<"\n";
}

std::array<char, 16> toArray(const std::string& str) {
    std::array<char, 16> arr{};
    std::strncpy(arr.data(), str.c_str(), 16);
    return arr;
}

int main() {
    net::io_context ioc;
    udp::endpoint endpoint(udp::v4(), 9000);
    udp::socket socket(ioc, endpoint);

    socket.set_option(net::ip::multicast::join_group(
        net::ip::make_address_v4("239.255.255.1"),
        net::ip::make_address_v4("192.168.29.226")
    ));

    std::array<char, 2048> data;  
    udp::endpoint sender_endpoint;

    while (true) {
        size_t len = socket.receive_from(net::buffer(data), sender_endpoint);

        MarketDataEventProto proto;
        if (!proto.ParseFromArray(data.data(), static_cast<int>(len))) {
            std::cerr << "Failed to parse protobuf\n";
            continue;
        }

        MarketDataEvent event;
        event.sequence = proto.sequence();
        event.symbol = toArray(proto.symbol());
        event.exchange1 = toArray(proto.exchange1());
        event.exchange2 = toArray(proto.exchange2());
        event.spread12 = proto.spread12();
        event.spread21 = proto.spread21();
        event.bestBidPrice = proto.bestbidprice();
        event.bestBidSize = proto.bestbidsize();
        event.bestBidExchange = toArray(proto.bestbidexchange());
        event.bestAskPrice = proto.bestaskprice();
        event.bestAskSize = proto.bestasksize();
        event.bestAskExchange = toArray(proto.bestaskexchange());
        event.imbalance1 = proto.imbalance1();
        event.imbalance2 = proto.imbalance2();
        event.aggImbalance = proto.aggimbalance();
        event.simpleMid1 = proto.simplemid1();
        event.simpleMid2 = proto.simplemid2();
        event.simpleDivergence = proto.simpledivergence();
        event.weightedMid1 = proto.weightedmid1();
        event.weightedMid2 = proto.weightedmid2();
        event.weightedDivergence = proto.weighteddivergence();

        coutEvent(event);
    }

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}