#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <cstdint>
#include <vector>
#include <queue>
#include <thread>

namespace net = boost::asio;
using udp = net::ip::udp;

class MulticastPublisher {
  public:
    MulticastPublisher();
    void start();
    void push(std::vector<uint8_t>& buffer);
    void publish(const std::vector<uint8_t>& buffer);

  private:
    net::io_context ioc;
    udp::endpoint multicast_endpoint;
    udp::socket socket;

    bool running = false;
    std::jthread worker;
    std::mutex multicast_mtx;
    std::condition_variable cv;
    std::queue<std::vector<uint8_t>> multicastQ;
};