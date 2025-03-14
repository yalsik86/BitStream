#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>
#include <iostream>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = asio::ssl;
using tcp = asio::ip::tcp;
namespace json = boost::json;

void connectToBinance(asio::io_context& ioc, ssl::context& ctx, websocket::stream<beast::ssl_stream<beast::tcp_stream>>& ws) {
    try {
        // Resolve Binance WebSocket address
        tcp::resolver resolver(ioc);
        auto const results = resolver.resolve("stream.binance.com", "9443");

        // Connect to Binance
        beast::get_lowest_layer(ws).connect(results);

        // Set SNI Hostname (important for some servers)
        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), "stream.binance.com")) {
            throw beast::system_error(beast::error_code(static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()), "Failed to set SNI");
        }

        // Perform SSL handshake
        ws.next_layer().handshake(ssl::stream_base::client);

        // Perform WebSocket handshake
        ws.handshake("stream.binance.com", "/ws");

        std::cout << "Connected to Binance WebSocket!" << std::endl << std::endl;

        // Subscribe to BTC-USD ticker
        json::value subscription = {
            {"method", "SUBSCRIBE"},
            {"params", json::array{"btcusdt@aggTrade"}},
            {"id", 1}
        };

        std::string message = json::serialize(subscription);
        ws.write(asio::buffer(message));

        std::cout << "Subscribed to BTC-USD ticker\n" << std::endl<< std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl << std::endl;
    }
}

void receiveFromBinance(websocket::stream<beast::ssl_stream<beast::tcp_stream>>& ws) {
    try
    {
        beast::flat_buffer buffer;
        int cnt = 0;
        while(cnt!=100) {
            ws.read(buffer);
            std::string data = beast::buffers_to_string(buffer.data());
            std::cout << "Received: " << data << std::endl << std::endl;
            buffer.clear();
            cnt++;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error receiving data: " << e.what() << std::endl << std::endl;
    }
    
}

void disconnectFromBinance(websocket::stream<beast::ssl_stream<beast::tcp_stream>>& ws) {
    try {
        ws.close(websocket::close_code::normal);
        std::cout << "Disconnected from Binance WebSocket." << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Disconnection error: " << e.what() << std::endl << std::endl;
    }
}

int main() {
    asio::io_context ioc;
    ssl::context ctx(ssl::context::tls_client);
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws(ioc, ctx);

    connectToBinance(ioc, ctx, ws);
    receiveFromBinance(ws);
    disconnectFromBinance(ws);

    return 0;
}