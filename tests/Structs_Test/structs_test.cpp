#include <iostream>
#include <type_traits>

#include "Structs/ExchangeUpdate.hpp"
#include "Structs/GlobalBBO.hpp"
#include "Structs/MarketDataEvent.hpp"
#include "Structs/NetImbalance.hpp"

// Verify trivial copyability at compile time.
static_assert(std::is_trivially_copyable_v<ExchangeUpdate>);
static_assert(std::is_trivially_copyable_v<GlobalBBO>);
static_assert(std::is_trivially_copyable_v<MarketDataEvent>);
static_assert(std::is_trivially_copyable_v<NetImbalance>);

// Verify default constructibility at compile time.
static_assert(std::is_default_constructible_v<ExchangeUpdate>);
static_assert(std::is_default_constructible_v<GlobalBBO>);
static_assert(std::is_default_constructible_v<MarketDataEvent>);
static_assert(std::is_default_constructible_v<NetImbalance>);

int main() {
    std::cout << "All structs are trivially copyable.\n";
    std::cout << "All structs are default constructible.\n\n";

    std::cout << "Struct sizes (bytes):\n";
    std::cout << "ExchangeUpdate: " << sizeof(ExchangeUpdate) << '\n';
    std::cout << "GlobalBBO: " << sizeof(GlobalBBO) << '\n';
    std::cout << "MarketDataEvent: " << sizeof(MarketDataEvent) << '\n';
    std::cout << "NetImbalance: " << sizeof(NetImbalance) << '\n';
}