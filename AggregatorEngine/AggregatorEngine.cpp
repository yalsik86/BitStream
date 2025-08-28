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
    std::unique_lock<std::mutex> lock(ingestion_mtx);
    snapshots[update.exchange] = update;

    for(auto& [otherEx, otherUpd]: snapshots) {
        if(otherEx==update.exchange) continue;

        MarketDataEvent event;
        event.symbol = update.symbol;

        event.exchange1 = update.exchange;
        event.exchange2 = otherEx;

        event.spread12 = otherUpd.askPrice - update.bidPrice;
        event.spread21 = update.askPrice - otherUpd.bidPrice;

        std::cout << std::fixed << std::setprecision(4)
        <<"["<<event.symbol<<"]\n"
        <<"["<<event.exchange1<<"] - ["<<event.exchange2<<"]\n"
        <<"| Spread12: "<<event.spread12<<" | Spread21: "<<event.spread21<<"\n";
    }
}