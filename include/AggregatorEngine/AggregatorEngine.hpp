#pragma once
#include "Protocol/market_data.pb.h"
#include "FeedHandlers/IExchangeFeed.hpp"
#include "DataDisseminator/DataDisseminator.hpp"
#include "Structs/MarketDataEvent.hpp"
#include "Structs/ExchangeUpdate.hpp"
#include "Structs/NetImbalance.hpp"
#include "Structs/GlobalBBO.hpp"
#include "Utils/StringUtils.hpp"
#include "Enums/ExchangeId.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <limits>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>

class AggregatorEngine {
  public:
    std::atomic<bool> run_flag{true};
    AggregatorEngine(DataDisseminator& router);
    void addConnector(std::unique_ptr<IExchangeFeed> conn);
    void start();
    void shutdown();
    void ingestRaw(const std::string& exchange, const std::string& rawData);
    void ingestUpdate(const ExchangeUpdate& update);

  private:
    DataDisseminator& disseminator;

    uint32_t seq = 0;

    std::array<ExchangeUpdate, EXCHANGE_COUNT> snapshots{}; // latest updates per-exchange
    std::vector<std::unique_ptr<IExchangeFeed>> connectors;
    std::vector<std::jthread> threads;
    std::mutex ingestion_mtx;

    // ------ Global BBO Utilities ------
    GlobalBBO globalBBOState{};
    void updateGlobalBBO(const ExchangeUpdate& update);

    // ------ Imbalance Utilities ------
    std::array<double, EXCHANGE_COUNT> exchangeImbalanceState{}; // per-exchange imbalance
    double computeImbalance(double bid, double ask);
    NetImbalance netImbalanceState{};
    void updateNetImbalance(const ExchangeUpdate& update);

    // ------ Mid-Price Utilities ------
    std::array<std::pair<double, double>, EXCHANGE_COUNT> exchangeMidPriceState{}; // per-exchange mid-price (simple, weighted)
    std::pair<double, double> computeMidPrice(const ExchangeUpdate& update);

    // ------ Event Utilities ------
    MarketDataEvent createEvent(
        const ExchangeUpdate& update, const ExchangeUpdate& otherUpd,
        double updateImbalance, const std::pair<double, double>& updateMidPrice
    ) const;
    
    void assignString(std::array<char, 16>& arr, const std::string& src) const;
    void coutEvent(const MarketDataEvent& event);
};