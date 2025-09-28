#pragma once
#include "Structs/RetransmitterEntry.hpp"
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <vector>
#include <limits>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace asio = boost::asio;
using udp = boost::asio::ip::udp;

class Retransmitter {
  public:
    Retransmitter();
    void start();
    void stop();
    void put(const uint32_t seq, const std::vector<uint8_t>& buffer);
    bool get(const uint32_t seq, std::vector<uint8_t>& resend_buffer);

  private:
    asio::io_context ioc;
    udp::endpoint retransmitter_endpoint;
    udp::socket socket;

    std::atomic<bool> running{false};
    std::jthread worker;
    std::mutex retransmitter_mtx;
    std::condition_variable cv;
    std::vector<RetransmitterEntry> retransmitBuffer;
};