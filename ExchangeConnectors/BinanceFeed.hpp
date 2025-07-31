#pragma once
#include "IExchangeFeed.hpp"

class BinanceFeed : public IExchangeFeed {
  public:
    void connect() override;
};