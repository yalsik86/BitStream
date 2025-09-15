#include "DataDisseminator/MulticastPublisher.hpp"
#include "TimedBuffer.hpp"
#include <algorithm>
#include <numeric>
#include <iostream>
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>

namespace lockfree = boost::lockfree;

class TestPublisher_lf : public MulticastPublisher {
  public:
    TestPublisher_lf();
    void start();
    void stop();
    void push(std::vector<uint8_t>& buffer);
    void publish(const std::vector<uint8_t>& buffer);

    void reportStats();

  private:
    std::atomic<bool> running{false};
    std::jthread worker;
    std::mutex test_mtx;
    std::condition_variable cv;
    lockfree::spsc_queue<TimedBuffer, lockfree::capacity<8192>> testQ;

    std::vector<long long> samples;
};