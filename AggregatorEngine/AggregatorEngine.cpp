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

void AggregatorEngine::ingestRaw(const std::string& exchange, const std::string& rawData) {
    std::cout<<exchange<<" "<<rawData<<"\n";
}

void AggregatorEngine::ingestUpdate(const ExchangeUpdate& update) {
    std::cout << std::fixed << std::setprecision(8)
        <<"["<<update.exchange<<"] "<<update.symbol<<"\n"
        <<"| Bid: "<<update.bidPrice<<" | Ask: "<<update.askPrice<<"\n";
}