#pragma once
#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <cstdint>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>

namespace lockfree = boost::lockfree;
namespace asio = boost::asio;
using udp = asio::ip::udp;

class MulticastPublisher {
  public:
    MulticastPublisher();
    void start();
    void stop();
    void push(std::vector<uint8_t>& buffer);
    void publish(const std::vector<uint8_t>& buffer);

  private:
    asio::io_context ioc;
    udp::endpoint multicast_endpoint;
    udp::socket socket;

    std::atomic<bool> running{false};
    std::jthread worker;
    std::mutex multicast_mtx;
    std::condition_variable cv;
    lockfree::spsc_queue<std::vector<uint8_t>, lockfree::capacity<8192>> multicastQ;
};