#pragma once

#include <chrono>
#include <string>
#include <utility>
#include <spdlog/spdlog.h>

class StageTimer {
public:
    explicit StageTimer(std::string stage) : stage_(std::move(stage)), start_(Clock::now()) {
        spdlog::info("[Timer] {} started", stage_);
    }

    ~StageTimer() {
        const auto elapsed = std::chrono::duration<double, std::milli>(Clock::now() - start_).count();
        spdlog::info("[Timer] {} completed in {:.2f} ms", stage_, elapsed);
    }

    StageTimer(const StageTimer&) = delete;
    StageTimer& operator=(const StageTimer&) = delete;
private:
    using Clock = std::chrono::steady_clock;

    std::string stage_;
    Clock::time_point start_;
};
