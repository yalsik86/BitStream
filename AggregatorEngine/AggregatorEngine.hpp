#pragma once
#include "../ExchangeConnectors/IExchangeFeed.hpp"
#include "../Structs/MarketDataEvent.hpp"
#include "../Structs/ExchangeUpdate.hpp"
#include "../Structs/NetImbalance.hpp"
#include "../Structs/GlobalBBO.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <thread>

class AggregatorEngine {
  public:
    void addConnector(std::unique_ptr<IExchangeFeed> conn);
    void start();
    void ingestRaw(const std::string& exchange, const std::string& rawData);
    void ingestUpdate(const ExchangeUpdate& update);

  private:
    std::unordered_map<std::string, ExchangeUpdate> snapshots; // latest updates per-exchange
    std::vector<std::unique_ptr<IExchangeFeed>> connectors;
    std::vector<std::jthread> threads;
    std::mutex ingestion_mtx;

    // ------ Global BBO Utilities ------
    GlobalBBO globalBBO;
    inline void updateGlobalBBO(const ExchangeUpdate& update);

    // ------ Imbalance Utilities ------
    std::unordered_map<std::string, double> exchangeImbalance; // per-exchange imbalance
    inline double computeImbalance(double bid, double ask);
    NetImbalance netImbalance;
    inline void updateNetImbalance(const ExchangeUpdate& update);

    // ------ Mid-Price Utilities ------
    std::unordered_map<std::string, std::pair<double, double>> exchangeMidPrice;
    inline std::pair<double, double> computeMidPrice(const ExchangeUpdate& update);

    // ------ Event Utilities ------
    inline MarketDataEvent createEvent(
        const ExchangeUpdate& update, const ExchangeUpdate& otherUpd,
        double updateImbalance, const std::pair<double, double>& updateMidPrice
    ) const;

    inline void assignString(std::array<char, 16>& arr, const std::string& src) const;
    
    inline void coutEvent(const MarketDataEvent& event);
};