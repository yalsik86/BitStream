#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>

enum class ExchangeId : std::uint8_t {
    Binance,
    Coinbase,
    Kraken,
    CryptoCom,

    Count
};

inline constexpr std::size_t EXCHANGE_COUNT = static_cast<std::size_t>(ExchangeId::Count);

constexpr std::string_view ExchangeIdToExchangeName(ExchangeId id) noexcept {
    switch (id) {
        case ExchangeId::Binance:
            return "Binance";
        case ExchangeId::Coinbase:
            return "Coinbase";
        case ExchangeId::Kraken:
            return "Kraken";
        case ExchangeId::CryptoCom:
            return "Crypto.com";
        default:
            return "Unknown";
    }
}

constexpr ExchangeId ExchangeNameToExchangeId(std::string_view name) noexcept {
    if (name == "Binance")
        return ExchangeId::Binance;
    if (name == "Coinbase")
        return ExchangeId::Coinbase;
    if (name == "Kraken")
        return ExchangeId::Kraken;
    if (name == "Crypto.com")
        return ExchangeId::CryptoCom;
    return ExchangeId::Count;
}

static_assert(EXCHANGE_COUNT <= 256);