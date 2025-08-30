#include "AggregatorEngine.hpp"

void AggregatorEngine::addConnector(std::unique_ptr<IExchangeFeed> conn) {
    connectors.push_back(std::move(conn));
}

void AggregatorEngine::start() {
    for(auto &conn: connectors) {
        threads.emplace_back(std::jthread([ptr = conn.get()]() {
            ptr->connect();
            ptr->receiveUpdates();
        }));
    }
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

void AggregatorEngine::ingestRaw(const std::string& exchange, const std::string& rawData) {
    std::cout<<exchange<<" "<<rawData<<"\n";
}

void AggregatorEngine::ingestUpdate(const ExchangeUpdate& update) {
    std::unique_lock<std::mutex> lock(ingestion_mtx);

    updateNetImbalance(update);
    snapshots[update.exchange] = update;
    updateGlobalBBO(update);

    double updateImbalance = computeImbalance(update.bidSize, update.askSize);
    exchangeImbalance[update.exchange] = updateImbalance;

    double updateMidPrice = (update.bidPrice + update.askPrice) / 2.0;
    exchangeMidPrice[update.exchange] = updateMidPrice;

    for(const auto& [otherEx, otherUpd]: snapshots) {
        if(otherEx==update.exchange) continue;

        MarketDataEvent event;
        event.symbol = update.symbol;

        event.exchange1 = update.exchange;
        event.exchange2 = otherEx;
        
        // Cross-exchange spread
        event.spread12 = otherUpd.askPrice - update.bidPrice;
        event.spread21 = update.askPrice - otherUpd.bidPrice;

        // Global BBO
        event.bestBidExchange = globalBBO.bestBidEx;
        event.bestBidPrice = globalBBO.bestBid;
        event.bestBidSize = globalBBO.bestBidSize;

        event.bestAskExchange = globalBBO.bestAskEx;
        event.bestAskPrice = globalBBO.bestAsk;
        event.bestAskSize = globalBBO.bestAskSize;

        // Top-of-Book Imbalance
        event.imbalance1 = updateImbalance;
        event.imbalance2 = exchangeImbalance[otherEx];

        event.aggImbalance = netImbalance.value;

        // Mid-Price, Divergence
        event.midPrice1 = updateMidPrice;
        event.midPrice2 = exchangeMidPrice[otherEx];
        // arbitrage indication
        event.midPriceDivergence = std::abs(event.midPrice1 - event.midPrice2);

        std::cout << std::fixed << std::setprecision(4)
        <<"------["<<event.symbol<<"]------\n"
        <<"["<<event.exchange1<<"] - ["<<event.exchange2<<"]\n"
        <<"| Spread12: "<<event.spread12<<" | Spread21: "<<event.spread21<<"\n"
        <<"| Global BBO: \n"
        <<"    - Best Bid: "<<event.bestBidPrice<<" - Size: "<<event.bestBidSize<<" @ "<<event.bestBidExchange<<"\n"
        <<"    - Best Ask: "<<event.bestAskPrice<<" - Size: "<<event.bestAskSize<<" @ "<<event.bestAskExchange<<"\n"
        <<"| Imbalance1: "<<event.imbalance1<<" | Imbalance2: "<<event.imbalance2<<"\n"
        <<"| Aggregate Imbalance: "<<event.aggImbalance<<"\n"
        <<"| Mid_Price1: "<<event.midPrice1<<" | Mid_Price2: "<<event.midPrice2<<"\n"
        <<"    - Divergence: "<<event.midPriceDivergence<<"\n";
    }
}