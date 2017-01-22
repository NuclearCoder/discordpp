//
// Created by aidan on 1/21/17.
//

#ifndef DISCORD_AIDOBOT_CLIENT_HH
#define DISCORD_AIDOBOT_CLIENT_HH

#include "discordpp.hh"

#include <string>
#include "lib/nlohmannjson/src/json.hpp"

#include <beast/websocket/ssl.hpp>
#include <beast/websocket.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace discordpp {
    using json = nlohmann::json;
    namespace asio = boost::asio;
    using boost::system::error_code;
    using stream_type = asio::ssl::stream<boost::asio::ip::tcp::socket&>;

    class Client {
    public:
        Client(asio::io_service& asio_ios, const std::string& token, std::map<std::string, std::function<void(json)>> eventResponses);
        ~Client() = default;

        Client(const Client &) = delete;

        Client &operator=(const Client &) = delete;

        std::map<std::string, std::function<void(json)>> eventResponses_;

    private:
        void onMessage(boost::system::error_code const &ec, beast::websocket::opcode *op, beast::streambuf *sb); //websocketpp::connection_hdl hdl, message_ptr msg

        void keepalive(uint32_t ms);

        std::string fetchGateway(std::string token = data::lastToken());

        //////////////////////////////
        asio::io_service &asio_ios_;
        std::string token_;
        asio::steady_timer keepalive_timer_;
        std::shared_ptr<beast::websocket::stream<stream_type&>> ws_;
    };

    /*class Client {
        using client = websocketpp::client<websocketpp::config::asio_tls_client>;
        using message_ptr = websocketpp::config::asio_client::message_type::ptr;
    public:
        Client(asio::io_service& asio_ios, const std::string& token, std::map<std::string, std::function<void(json)>> eventResponses);//std::map <std::string, std::string> soft_responses = {})
        ~Client() =default;
        Client(const Client&) =delete;
        Client& operator=(const Client&) =delete;
        std::map<std::string, std::function<void(json)>> eventResponses_;

    private:
        void on_message(websocketpp::connection_hdl hdl, message_ptr msg);

        void keepalive(uint32_t ms);

        void on_open(websocketpp::connection_hdl hdl);
        std::string fetchGateway(std::string token = data::lastToken());
        //////////////////////////////
        asio::io_service& asio_ios_;
        std::string token_;
        client client_;
        websocketpp::uri_ptr uri_ptr_;
        client::connection_ptr connection_;
        asio::steady_timer keepalive_timer_;
    };*/
}

#endif //DISCORD_AIDOBOT_CLIENT_HH
