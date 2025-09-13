#include "AggregatorEngine.hpp"

AggregatorEngine::AggregatorEngine(MarketDataRouter& router) : router(router) {}

void AggregatorEngine::addConnector(std::unique_ptr<IExchangeFeed> conn) {
    connectors.push_back(std::move(conn));
}

void AggregatorEngine::start() {
    router.start();

    for(auto &conn: connectors) {
        threads.emplace_back(std::jthread([this, ptr = conn.get()]() {
            ptr->run(run_flag);
        }));
    }
}

void AggregatorEngine::shutdown() {
    spdlog::info("[Aggregator Engine] Shutting down connectors...");
    run_flag = false;
    for(auto& t: threads) {
        if(t.joinable()) t.join();
    }
    for(auto &conn: connectors) {
        conn->disconnect();
    }

    spdlog::info("[Aggregator Engine] Shutting down router...");
    router.shutdown();
}

inline void AggregatorEngine::updateGlobalBBO(const ExchangeUpdate& update) {
    // Best Bid update
    if(update.bidPrice > globalBBO.bestBid) {
        globalBBO.bestBid = update.bidPrice;
        globalBBO.bestBidSize = update.bidSize;
        globalBBO.bestBidEx = update.exchange;
    } 
    else if(update.exchange == globalBBO.bestBidEx) {
        globalBBO.bestBid = -1.0;
        for (auto& [ex, upd] : snapshots) {
            if (upd.bidPrice > globalBBO.bestBid) {
                globalBBO.bestBid = upd.bidPrice;
                globalBBO.bestBidSize = upd.bidSize;
                globalBBO.bestBidEx = ex;
            }
        }
    }

    // Best Ask update
    if(update.askPrice < globalBBO.bestAsk) {
        globalBBO.bestAsk = update.askPrice;
        globalBBO.bestAskSize = update.askSize;
        globalBBO.bestAskEx = update.exchange;
    } 
    else if(update.exchange == globalBBO.bestAskEx) {
        globalBBO.bestAsk = std::numeric_limits<double>::max();
        for (auto& [ex, upd] : snapshots) {
            if (upd.askPrice < globalBBO.bestAsk) {
                globalBBO.bestAsk = upd.askPrice;
                globalBBO.bestAskSize = upd.askSize;
                globalBBO.bestAskEx = ex;
            }
        }
    }
}

inline double AggregatorEngine::computeImbalance(double bidSize, double askSize) {
    return (bidSize + askSize != 0.0) ? (bidSize - askSize) / (bidSize + askSize) : 0.0;
}

inline void AggregatorEngine::updateNetImbalance(const ExchangeUpdate& update) {
    auto it = snapshots.find(update.exchange);
    if(it != snapshots.end()) {
        netImbalance.totalBid -= it->second.bidSize;
        netImbalance.totalAsk -= it->second.askSize;
    }

    netImbalance.totalBid += update.bidSize;
    netImbalance.totalAsk += update.askSize;

    netImbalance.value = computeImbalance(netImbalance.totalBid, netImbalance.totalAsk);
}

inline std::pair<double, double> AggregatorEngine::computeMidPrice(const ExchangeUpdate& update) {
    double updateSimpleMid = (update.bidPrice + update.askPrice) / 2.0;

    double denominator = update.bidSize + update.askSize;
    double updateWeightedMid = denominator > 0 
        ? (update.bidPrice * update.askSize + update.askPrice * update.bidSize) / denominator
        : updateSimpleMid;

    return std::make_pair(updateSimpleMid, updateWeightedMid);
}

void AggregatorEngine::ingestRaw(const std::string& exchange, const std::string& rawData) {
    std::cout<<exchange<<" "<<rawData<<"\n";
}

inline MarketDataEvent AggregatorEngine::createEvent(
    const ExchangeUpdate& update, const ExchangeUpdate& otherUpd,
    double updateImbalance, const std::pair<double, double>& updateMidPrice
) const {
    MarketDataEvent event;
    assignString(event.symbol, update.symbol);

    assignString(event.exchange1, update.exchange);
    assignString(event.exchange2, otherUpd.exchange);

    // Cross-exchange spread
    event.spread12 = otherUpd.askPrice - update.bidPrice;
    event.spread21 = update.askPrice - otherUpd.bidPrice;

    // Global BBO
    assignString(event.bestBidExchange, globalBBO.bestBidEx);
    event.bestBidPrice = globalBBO.bestBid;
    event.bestBidSize = globalBBO.bestBidSize;

    assignString(event.bestAskExchange, globalBBO.bestAskEx);
    event.bestAskPrice = globalBBO.bestAsk;
    event.bestAskSize = globalBBO.bestAskSize;

    // Top-of-Book Imbalance
    event.imbalance1 = updateImbalance;
    event.imbalance2 = exchangeImbalance.at(otherUpd.exchange);

    event.aggImbalance = netImbalance.value;

    // Mid-Price, Divergence
    const auto& [updateSimpleMid, updateWeightedMid] = updateMidPrice;
    const auto& [otherSimpleMid, otherWeightedMid] = exchangeMidPrice.at(otherUpd.exchange);
    event.simpleMid1 = updateSimpleMid;
    event.simpleMid2 = otherSimpleMid;

    event.weightedMid1 = updateWeightedMid;
    event.weightedMid2 = otherWeightedMid;
    // arbitrage indication
    event.simpleDivergence = std::abs(event.simpleMid1 - event.simpleMid2);
    event.weightedDivergence = std::abs(event.weightedMid1 - event.weightedMid2);

    return event;
}

void AggregatorEngine::ingestUpdate(const ExchangeUpdate& update) {
    std::unique_lock<std::mutex> lock(ingestion_mtx);

    updateNetImbalance(update);
    snapshots[update.exchange] = update;
    updateGlobalBBO(update);

    double updateImbalance = computeImbalance(update.bidSize, update.askSize);
    exchangeImbalance[update.exchange] = updateImbalance;

    auto updateMidPrice = computeMidPrice(update);
    exchangeMidPrice[update.exchange] = updateMidPrice;

    for(const auto& [otherEx, otherUpd]: snapshots) {
        if(otherEx==update.exchange) continue;

        MarketDataEvent event = createEvent(
            update, otherUpd,
            updateImbalance, 
            updateMidPrice
        );
        event.sequence = seq++;

        // coutEvent(event);
        router.ingestEvent(event);
    }
}

void AggregatorEngine::assignString(std::array<char, 16>& arr, const std::string& src) const {
    std::memset(arr.data(), 0, arr.size());
    std::strncpy(arr.data(), src.c_str(), arr.size() - 1);
}

inline void AggregatorEngine::coutEvent(const MarketDataEvent& event) {
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