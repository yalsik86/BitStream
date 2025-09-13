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
        while(running) {
            std::vector<uint8_t> buffer;
            {
                std::unique_lock<std::mutex> lock(multicast_mtx);
                cv.wait(lock, [&]() {
                    return !multicastQ.empty() || !running;
                });
                if(!running) break;

                buffer = std::move(multicastQ.front());
                multicastQ.pop();
            }

            publish(buffer);
        }
    });
}

void MulticastPublisher::stop() {
    running = false;
    cv.notify_all();
    if(worker.joinable()) {
        worker.join();
        spdlog::info("[Multicast Publisher] Worker thread exited cleanly");
    }
}

void MulticastPublisher::push(std::vector<uint8_t>& buffer) {
    {
        std::lock_guard<std::mutex> lock(multicast_mtx);
        multicastQ.push(std::move(buffer));
    }
    cv.notify_one();
}

void MulticastPublisher::publish(const std::vector<uint8_t>& buffer) {
    socket.send_to(asio::buffer(buffer), multicast_endpoint);
}