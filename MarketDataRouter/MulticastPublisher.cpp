#include "MulticastPublisher.hpp"

MulticastPublisher::MulticastPublisher() : socket(ioc, udp::endpoint(udp::v4(), 0)), multicast_endpoint(net::ip::make_address_v4("239.255.255.1"), 9000) {
    socket.set_option(net::ip::multicast::hops(5));
    socket.set_option(net::ip::multicast::enable_loopback(true));
    net::ip::address_v4 local_if = net::ip::make_address_v4("192.168.29.222");
    socket.set_option(net::ip::multicast::outbound_interface(local_if));
}

void MulticastPublisher::publish(const std::vector<uint8_t>& buffer) {
    socket.send_to(net::buffer(buffer), multicast_endpoint);
}