#include <cstdint>

enum class ExchangeId : uint8_t {
    Binance = 0,
    Coinbase = 1,
    Kraken = 2,
    CryptoCom = 3,
    
    Count = 4
};