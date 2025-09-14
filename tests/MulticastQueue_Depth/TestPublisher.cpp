#include "TestPublisher.hpp"

TestPublisher::TestPublisher() : MulticastPublisher() {}

void TestPublisher::start() {
    running = true;
    worker = std::jthread([this]() {
        spdlog::info("[Multicast Publisher] Worker thread started");
        while(running) {
            // sample queue size
            samples.push_back((long long)testQ.size());
            
            std::vector<uint8_t> buffer;
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                cv.wait(lock, [&]() {
                    return !testQ.empty() || !running;
                });
                if(!running) break;

                buffer = std::move(testQ.front());
                testQ.pop();
            }

            publish(buffer);
        }
    });
}

void TestPublisher::stop() {
    running = false;
    cv.notify_all();
    if(worker.joinable()) {
        worker.join();
    }
    spdlog::info("[Multicast Publisher] Worker thread exited cleanly");
}

void TestPublisher::push(std::vector<uint8_t>& buffer) {
    {
        std::lock_guard<std::mutex> lock(test_mtx);
        testQ.push(std::move(buffer));
        // sample queue size
        samples.push_back((long long)testQ.size());
    }
    cv.notify_one();
}

void TestPublisher::reportStats() {
    if(samples.empty()) {
        std::cout << "No samples collected\n";
        return;
    }
    
    std::sort(samples.begin(), samples.end());

    auto min = samples.front();
    auto max = samples.back();

    double avg = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();

    auto p95 = samples[samples.size() * 95 / 100];
    auto p99 = samples[samples.size() * 99 / 100];

    std::cout << "Queue depth stats\n"
          << "avg: " << avg << " msgs\n"
          << "p95: " << p95 << " msgs\n"
          << "p99: " << p99 << " msgs\n"
          << "max: " << max << " msgs\n";
}
