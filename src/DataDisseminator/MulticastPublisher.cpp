#include "DataDisseminator/MulticastPublisher.hpp"

MulticastPublisher::MulticastPublisher() : socket(ioc, udp::endpoint(udp::v4(), 0)), multicast_endpoint(asio::ip::make_address_v4("239.255.255.1"), 9000) {
    socket.set_option(asio::ip::multicast::hops(5));
    socket.set_option(asio::ip::multicast::enable_loopback(true));
    asio::ip::address_v4 local_if = asio::ip::make_address_v4("192.168.29.222");
    socket.set_option(asio::ip::multicast::outbound_interface(local_if));
}

void MulticastPublisher::start() {
    running = true;
    worker = std::jthread([this]() {
        spdlog::info("[Multicast Publisher] Worker thread started");
        std::vector<uint8_t> buffer;
        buffer.reserve(256);
        
        while(running) {
            while(!multicastQ.pop(buffer) && running) {
                // spin-wait
            }
            if(!running) break;

            publish(buffer);
        }
    });
}

void MulticastPublisher::stop() {
    running = false;
    if(worker.joinable()) {
        worker.join();
    }
    multicastQ.reset();
    spdlog::info("[Multicast Publisher] Worker thread exited cleanly");
}

void MulticastPublisher::push(std::vector<uint8_t>& buffer) {
    while(!multicastQ.push(std::move(buffer))) {
        // spin-wait
    }
}

void MulticastPublisher::publish(const std::vector<uint8_t>& buffer) {
    socket.send_to(asio::buffer(buffer), multicast_endpoint);
}