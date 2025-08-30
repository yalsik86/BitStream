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

    GlobalBBO globalBBO;
    inline void updateGlobalBBO(const ExchangeUpdate& update);

    std::unordered_map<std::string, double> exchangeImbalance;
    inline double computeImbalance(double bid, double ask);

    NetImbalance netImbalance;
    inline void updateNetImbalance(const ExchangeUpdate& update);

    std::unordered_map<std::string, std::pair<double, double>> exchangeMidPrice;
    inline std::pair<double, double> computeMidPrice(const ExchangeUpdate& update);
};