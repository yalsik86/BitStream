#include "DataDisseminator/Retransmitter.hpp"

Retransmitter::Retransmitter() : socket(ioc, udp::endpoint(udp::v4(), 0)), retransmitter_endpoint(asio::ip::make_address_v4("192.168.29.223"), 8080) {
    retransmitBuffer.resize(4096);
    socket.set_option(asio::ip::unicast::hops(5));
}

void Retransmitter::start() {
    running = true;
    worker = std::jthread([this]() {
        spdlog::info("[Retransmitter] Worker thread started");
        std::array<uint8_t, 256> receive_buffer;
        std::vector<uint8_t> resend_buffer;
        resend_buffer.reserve(256);

        while(running) {            
            try {
                udp::endpoint sender_endpoint;
                size_t bytes = socket.receive_from(asio::buffer(receive_buffer), sender_endpoint);

                if(bytes!=sizeof(uint32_t)) {
                    spdlog::warn("Received invalid request ({} bytes)", bytes);
                    continue;
                }
                if(!running) break;
                
                uint32_t sequence;
                memcpy(&sequence, receive_buffer.data(), sizeof(uint32_t));
                if(!get(sequence, resend_buffer)) {
                    continue;
                }
                socket.send_to(asio::buffer(resend_buffer), sender_endpoint);
                spdlog::info("[Retransmitter] Resent payload with sequence number: {}", sequence);
            } catch (const boost::system::system_error& e) {
                spdlog::warn("[Retransmitter] receive_from threw: {}", e.code().message());
                continue;
            }
        }
    });
}

void Retransmitter::stop() {
    running = false;
    socket.cancel();
    socket.close();
    if(worker.joinable()) worker.join();
    spdlog::info("[Retransmitter] Worker thread exited cleanly");
}

void Retransmitter::put(const uint32_t seq, const std::vector<uint8_t>& buffer) {
    std::lock_guard<std::mutex> lock(retransmitter_mtx);
    auto& entry = retransmitBuffer[seq % 4096];
    entry.sequence = seq;
    entry.payload = buffer;
}

bool Retransmitter::get(const uint32_t seq, std::vector<uint8_t>& resend_buffer) {
    std::lock_guard<std::mutex> lock(retransmitter_mtx);
    auto& entry = retransmitBuffer[seq % 4096];
    if(entry.sequence == seq) {
        resend_buffer = entry.payload;
        return true;
    }
    return false;
}