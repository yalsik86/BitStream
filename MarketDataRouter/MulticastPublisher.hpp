#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <cstdint>
#include <vector>

namespace net = boost::asio;
using udp = net::ip::udp;

class MulticastPublisher {
  public:
    MulticastPublisher();
    void publish(const std::vector<uint8_t>& buffer);

  private:
    net::io_context ioc;
    udp::endpoint multicast_endpoint;
    udp::socket socket;
};