#include "AggregatorEngine/AggregatorEngine.hpp"

AggregatorEngine::AggregatorEngine(DataDisseminator& disseminator) : disseminator(disseminator) {}

void AggregatorEngine::addConnector(std::unique_ptr<IExchangeFeed> conn) {
    connectors.push_back(std::move(conn));
}

void AggregatorEngine::start() {
    disseminator.start();

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

    spdlog::info("[Aggregator Engine] Shutting down disseminator...");
    disseminator.shutdown();
}

void AggregatorEngine::updateGlobalBBO(const ExchangeUpdate& update) {
    // Best Bid update
    if(update.bidPrice > globalBBOState.bestBid) { // update new best bid
        globalBBOState.bestBid = update.bidPrice;
        globalBBOState.bestBidSize = update.bidSize;
        globalBBOState.bestBidExchangeId = update.exchangeId;
    } 
    else if(update.exchangeId == globalBBOState.bestBidExchangeId) { // re-evaluate from snapshots if best bid exchange updated it's best bid
        globalBBOState.bestBid = -1.0;
        globalBBOState.bestBidSize = 0.0;
        globalBBOState.bestBidExchangeId = ExchangeId::Count;

        for (auto& snapshot : snapshots) {
            if (snapshot.hasReceivedUpdate && snapshot.bidPrice > globalBBOState.bestBid) {
                globalBBOState.bestBid = snapshot.bidPrice;
                globalBBOState.bestBidSize = snapshot.bidSize;
                globalBBOState.bestBidExchangeId = snapshot.exchangeId;
            }
        }
    }

    // Best Ask update
    if(update.askPrice < globalBBOState.bestAsk) { // update new best ask
        globalBBOState.bestAsk = update.askPrice;
        globalBBOState.bestAskSize = update.askSize;
        globalBBOState.bestAskExchangeId = update.exchangeId;
    } 
    else if(update.exchangeId == globalBBOState.bestAskExchangeId) { // re-evaluate from snapshots if best ask exchange updated it's best ask
        globalBBOState.bestAsk = std::numeric_limits<double>::max();
        globalBBOState.bestAskSize = 0.0;
        globalBBOState.bestAskExchangeId = ExchangeId::Count;

        for (auto& snapshot : snapshots) {
            if (snapshot.hasReceivedUpdate && snapshot.askPrice < globalBBOState.bestAsk) {
                globalBBOState.bestAsk = snapshot.askPrice;
                globalBBOState.bestAskSize = snapshot.askSize;
                globalBBOState.bestAskExchangeId = snapshot.exchangeId;
            }
        }
    }
}

double AggregatorEngine::computeImbalance(double bidSize, double askSize) {
    return (bidSize + askSize != 0.0) ? (bidSize - askSize) / (bidSize + askSize) : 0.0;
}

void AggregatorEngine::updateNetImbalance(const ExchangeUpdate& update) {
    // to index into AggregatorEngine snapshots state 
    const auto updateIndex = ExchangeIdToIndex(update.exchangeId);

    const auto& previous = snapshots[updateIndex];
    if(previous.hasReceivedUpdate) {
        netImbalanceState.totalBid -= previous.bidSize;
        netImbalanceState.totalAsk -= previous.askSize;
    }

    netImbalanceState.totalBid += update.bidSize;
    netImbalanceState.totalAsk += update.askSize;

    netImbalanceState.value = computeImbalance(netImbalanceState.totalBid, netImbalanceState.totalAsk);
}

std::pair<double, double> AggregatorEngine::computeMidPrice(const ExchangeUpdate& update) {
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

MarketDataEvent AggregatorEngine::createEvent(
    const ExchangeUpdate& update, const ExchangeUpdate& snapshot,
    double updateImbalance, const std::pair<double, double>& updateMidPrice
) const {
    MarketDataEvent event{};

    // get index into AggregatorEngine state arrays
    const auto snapshotIndex = ExchangeIdToIndex(snapshot.exchangeId);

    event.symbol = update.symbol;
    StringUtils::assignStringToArray(event.exchange1, ExchangeIdToExchangeName(update.exchangeId));
    StringUtils::assignStringToArray(event.exchange2, ExchangeIdToExchangeName(snapshot.exchangeId));

    // Cross-exchange spread
    event.spread12 = snapshot.askPrice - update.bidPrice;
    event.spread21 = update.askPrice - snapshot.bidPrice;

    // Global BBO
    StringUtils::assignStringToArray(event.bestBidExchange, ExchangeIdToExchangeName(globalBBOState.bestBidExchangeId));
    event.bestBidPrice = globalBBOState.bestBid;
    event.bestBidSize = globalBBOState.bestBidSize;

    StringUtils::assignStringToArray(event.bestAskExchange, ExchangeIdToExchangeName(globalBBOState.bestAskExchangeId));
    event.bestAskPrice = globalBBOState.bestAsk;
    event.bestAskSize = globalBBOState.bestAskSize;

    // Top-of-Book Imbalance
    event.imbalance1 = updateImbalance;
    event.imbalance2 = exchangeImbalanceState[snapshotIndex];

    event.aggImbalance = netImbalanceState.value;

    // Mid-Price, Divergence
    const auto& [updateSimpleMid, updateWeightedMid] = updateMidPrice;
    const auto& [snapshotSimpleMid, snapshotWeightedMid] = exchangeMidPriceState[snapshotIndex];
    event.simpleMid1 = updateSimpleMid;
    event.simpleMid2 = snapshotSimpleMid;

    event.weightedMid1 = updateWeightedMid;
    event.weightedMid2 = snapshotWeightedMid;
    // arbitrage indication
    event.simpleDivergence = std::abs(event.simpleMid1 - event.simpleMid2);
    event.weightedDivergence = std::abs(event.weightedMid1 - event.weightedMid2);

    return event;
}

void AggregatorEngine::ingestUpdate(const ExchangeUpdate& update) {
    if (update.exchangeId >= ExchangeId::Count) {
        return;
    }
    std::unique_lock<std::mutex> lock(ingestion_mtx);

    updateNetImbalance(update);
    
    const auto updateIndex = ExchangeIdToIndex(update.exchangeId);
    snapshots[updateIndex] = update;
    snapshots[updateIndex].hasReceivedUpdate = true;

    updateGlobalBBO(update);

    double updateImbalance = computeImbalance(update.bidSize, update.askSize);
    exchangeImbalanceState[updateIndex] = updateImbalance;

    auto updateMidPrice = computeMidPrice(update);
    exchangeMidPriceState[updateIndex] = updateMidPrice;

    for(const auto& snapshot: snapshots) {
        if(!snapshot.hasReceivedUpdate || snapshot.exchangeId == update.exchangeId) continue;

        MarketDataEvent event = createEvent(
            update, snapshot,
            updateImbalance, 
            updateMidPrice
        );
        event.sequence = seq++;

        // coutEvent(event);
        disseminator.ingestEvent(event);
    }
}

void AggregatorEngine::assignString(std::array<char, 16>& arr, const std::string& src) const {
    std::memset(arr.data(), 0, arr.size());
    std::strncpy(arr.data(), src.c_str(), arr.size() - 1);
}

void AggregatorEngine::coutEvent(const MarketDataEvent& event) {
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