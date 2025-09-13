#include "DataDisseminator/MulticastPublisher.hpp"
#include "TimedBuffer.hpp"
#include <algorithm>

class TestPublisher : public MulticastPublisher {
  public:
    TestPublisher();
    void start();
    void stop();
    void push(std::vector<uint8_t>& buffer);
    void publish(const std::vector<uint8_t>& buffer);

    void reportStats();

  private:
    bool running{false};
    std::jthread worker;
    std::mutex test_mtx;
    std::condition_variable cv;
    std::queue<TimedBuffer> testQ;

    std::vector<long long> samples;
};