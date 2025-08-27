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