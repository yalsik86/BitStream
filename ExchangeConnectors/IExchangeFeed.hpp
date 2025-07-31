#pragma once

#include <iostream>

class IExchangeFeed {
  public:
    virtual ~IExchangeFeed() = default;

    virtual void connect() = 0;
};