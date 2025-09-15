#pragma once
#include <vector>
#include <chrono>

struct TimedBuffer {
    std::vector<uint8_t> data;
    std::chrono::high_resolution_clock::time_point enqueue_ts;

    TimedBuffer() {
        data.reserve(256); // safe headroom over observed 236
    }
};