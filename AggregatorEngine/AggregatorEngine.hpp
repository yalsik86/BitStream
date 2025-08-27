#pragma once
#include "../ExchangeConnectors/IExchangeFeed.hpp"
#include "../Structs/ExchangeUpdate.hpp"
#include <thread>

class AggregatorEngine {
  public:
    void addConnector(std::unique_ptr<IExchangeFeed> conn);
    void start();
    void ingestRaw(const std::string& exchange, const std::string& rawData);
    void ingestUpdate(const ExchangeUpdate& update);

  private:
    std::vector<std::unique_ptr<IExchangeFeed>> connectors;
    std::vector<std::jthread> threads;
};