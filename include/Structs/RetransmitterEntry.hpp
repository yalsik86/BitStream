#pragma once
#include <stdint.h>
#include <vector>

struct RetransmitterEntry {
    uint32_t sequence;
    std::vector<uint8_t> payload;

    RetransmitterEntry() { 
        payload.reserve(236); 
    }
};
