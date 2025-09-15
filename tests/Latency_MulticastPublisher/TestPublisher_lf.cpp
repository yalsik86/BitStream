#include "TestPublisher_lf.hpp"

TestPublisher_lf::TestPublisher_lf() : MulticastPublisher() {}

void TestPublisher_lf::start() {
    running = true;
    worker = std::jthread([this]() {
        spdlog::info("[Multicast Publisher] Worker thread started");
        TimedBuffer tb;

        while(running) {
            while(!testQ.pop(tb) && running) {
                // spin-wait
            }
            if(!running) break;

            MulticastPublisher::publish(tb.data);

            auto dequeue_ts = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>
                            (dequeue_ts - tb.enqueue_ts).count();

            samples.push_back(latency);
        }
    });
}

void TestPublisher_lf::push(std::vector<uint8_t>& buffer) {
    TimedBuffer tb;
    tb.data = std::move(buffer);
    tb.enqueue_ts = std::chrono::high_resolution_clock::now();
    while(!testQ.push(std::move(tb))){
        // spin-wait
    }
}

void TestPublisher_lf::stop() {
    running = false;
    if(worker.joinable()) {
        worker.join();
    }
    testQ.reset();
}

void TestPublisher_lf::reportStats() {
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

    std::cout << "Latency stats\n"
              << "min: " << min << " us\n"
              << "avg: " << avg << " us\n"
              << "p95: " << p95 << " us\n"
              << "p99: " << p99 << " us\n"
              << "max: " << max << " us\n";
}
