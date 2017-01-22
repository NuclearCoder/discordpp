//
// Created by aidan on 1/21/17.
//

#include "discordpp/client.hh"

#include <discordpp/discordpp.hh>
#include <beast/core.hpp>

using namespace discordpp;

namespace asio = boost::asio;
using boost::system::error_code;
//using socket = boost::asio::ip::tcp::socket;
//using resolver = boost::asio::ip::tcp::resolver;

Client::Client(asio::io_service& asio_ios, const std::string& token, std::map<std::string, std::function<void(json)>> eventResponses)
        : asio_ios_(asio_ios)
        , token_(token)
        , keepalive_timer_(asio_ios)
        , eventResponses_(eventResponses)
{
    //client_.set_access_channels(websocketpp::log::alevel::all);
    //client_.clear_access_channels(websocketpp::log::alevel::frame_payload);

    //client_.set_tls_init_handler([this](websocketpp::connection_hdl){
    //    return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);
    //});

    //client_.init_asio(&asio_ios);

    //client_.set_message_handler(std::bind(&Client::on_message, this,
    //                                      std::placeholders::_1, std::placeholders::_2));
    //client_.set_open_handler(std::bind(&Client::on_open, this,
    //                                   std::placeholders::_1));

    //websocketpp::lib::error_code ec;
    //std::string uri = fetchGateway(token);
    //std::cout << "Connecting to gateway at " << uri << "\n";
    //connection_ = client_.get_connection(uri, ec);
    //if (ec) {
    //    std::cout << "could not create connection because: " << ec.message() << std::endl;
    //    //TODO TBD: throw something
    //} else {
    //    // Note that connect here only requests a connection. No network messages are
    //    // exchanged until the event loop starts running in the next line.
    //    client_.connect(connection_);
    //}

    //boost::asio::ssl::context ctx{boost::asio::ssl::context::sslv23};
    //beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ws{asio_ios_, ctx};

    /*// Normal boost::asio setup
    std::string host = fetchGateway(token);
    std::cout << "Connecting to gateway at " << host << "\n";
    boost::asio::ip::tcp::resolver r{asio_ios_};
    boost::asio::ip::tcp::socket sock{asio_ios_};
    boost::asio::connect(sock, r.resolve(boost::asio::ip::tcp::resolver::query{host, "http"}));

    // Perform SSL handshaking
    asio::ssl::context ctx{asio::ssl::context::sslv23};
    stream_type stream{sock, ctx};
    stream.set_verify_mode(asio::ssl::verify_none);
    stream.handshake(asio::ssl::stream_base::client);

    // Secure WebSocket connect and send message using Beast
    ws_ = std::make_shared<beast::websocket::stream<stream_type&>>(stream);
    //ws_ = &ws;
    std::cout << "handshaking at " << host << "\n";
    ws_->handshake(host, "/");
    //ws.write(asio::buffer("Hello, world!"));*/

    // Normal boost::asio setup
    std::string host = fetchGateway(token);
    std::string::size_type protocolPos = host.find("wss://");
    if (protocolPos != std::string::npos) {
        host.erase(protocolPos, 6);
    }
    boost::asio::ip::tcp::resolver r{asio_ios_};
    boost::asio::ip::tcp::socket sock{asio_ios_};
    boost::asio::connect(sock,
                         r.resolve(boost::asio::ip::tcp::resolver::query{host, "http"}));

    // WebSocket connect and send message using beast
    std::cout << "here" << std::endl;
    beast::websocket::stream<boost::asio::ip::tcp::socket&> ws{sock};
    std::cout << "here" << std::endl;
    ws.handshake(host, "/");

    std::cout << "Connection established.\n";

    json connect = {
            {"op", 2},
            {"d", {
                           {"token", token_},
                           {"v", 4},
                           {"properties", {
                                                  {"$os", "linux"},
                                                  {"$browser", "discordpp"},
                                                  {"$device", "discordpp"},
                                                  {"$referrer",""}, {"$referring_domain",""}
                                          }
                           },
                           {"compress", false},
                           {"large_threshold", 250}
                   }
            }
    };
    std::cout << "Client Handshake:\n" << connect.dump(1) << "\n";

    ws_->write(asio::buffer(connect.dump()));

    // Receive Secure WebSocket message, print and close using Beast
    beast::streambuf *sb;
    beast::websocket::opcode *op;
    ws_->async_read(*op, *sb, std::bind(&Client::onMessage, this, std::placeholders::_1, op, sb));
}

void Client::onMessage(boost::system::error_code const &ec, beast::websocket::opcode *op, beast::streambuf *sb) { //websocketpp::connection_hdl hdl, message_ptr msg
    std::cout << json::parse(beast::to_string(sb->data())).dump(4);
    /*json jmessage = {};//json::parse(msg->get_payload());
    if(jmessage["op"].get<int>() == 0){ //Dispatch
        std::map<std::string, std::function<void(json)>>::iterator it = eventResponses_.find(jmessage["t"]);
        if(it != eventResponses_.end()){
            asio_ios_.post(std::bind(it->second, jmessage));
        } else {
            std::cout << "There is no function for the event " << jmessage["t"] << ".\n";
        }
        if(jmessage["t"] == "READY") {
            uint32_t ms = jmessage["d"]["heartbeat_interval"];
            ms *= .9;
            keepalive(ms);
        }
    } else if(jmessage["op"].get<int>() == 1){ //Heartbeat (This isn't implemented yet, still using periodic heartbeats for now.)
        //client_.send(hdl, jmessage.dump(), websocketpp::frame::opcode::text);
    } else { //Wat
        std::cout << "Unexpected opcode received:\n\n" << jmessage.dump(4) << "\n\n\n";
    }*/
    delete sb;
    delete op;
}

void Client::keepalive(uint32_t ms){
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    json message = {
            {"op", 1},
            {"d", millis}
    };
    std::cout << "Sending Heartbeat:\n";

    //client_.send(connection_, message.dump(), websocketpp::frame::opcode::text);
    //reset timer
    keepalive_timer_.expires_from_now(std::chrono::milliseconds(ms));
    keepalive_timer_.async_wait(std::bind(&Client::keepalive, this, ms));
}

/*
void Client::on_open(){ //websocketpp::connection_hdl hdl
    std::cout << "Connection established.\n";

    json connect = {
            {"op", 2},
            {"d", {
                           {"token", token_},
                           {"v", 4},
                           {"properties", {
                                                  {"$os", "linux"},
                                                  {"$browser", "discordpp"},
                                                  {"$device", "discordpp"},
                                                  {"$referrer",""}, {"$referring_domain",""}
                                          }
                           },
                           {"compress", false},
                           {"large_threshold", 250}
                   }
            }
    };
    std::cout << "Client Handshake:\n" << connect.dump(1) << "\n";
    //client_.send(hdl, connect.dump(), websocketpp::frame::opcode::text);
}
 */

std::string Client::fetchGateway(std::string token){
    return DiscordAPI::call("/gateway", token).at("url");
}

/*using client = websocketpp::client<websocketpp::config::asio_tls_client>;
using message_ptr = websocketpp::config::asio_client::message_type::ptr;
Client::Client(asio::io_service& asio_ios, const std::string& token, std::map<std::string, std::function<void(json)>> eventResponses)//std::map <std::string, std::string> soft_responses = {})
: asio_ios_(asio_ios)
, token_(token)
, keepalive_timer_(asio_ios)
, eventResponses_(eventResponses)
{
    client_.set_access_channels(websocketpp::log::alevel::all);
    client_.clear_access_channels(websocketpp::log::alevel::frame_payload);

    client_.set_tls_init_handler([this](websocketpp::connection_hdl){
        return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);
    });

    client_.init_asio(&asio_ios);

    client_.set_message_handler(std::bind(&Client::on_message, this,
                                          std::placeholders::_1, std::placeholders::_2));
    client_.set_open_handler(std::bind(&Client::on_open, this,
                                       std::placeholders::_1));

    websocketpp::lib::error_code ec;
    std::string uri = fetchGateway(token);
    std::cout << "Connecting to gateway at " << uri << "\n";
    connection_ = client_.get_connection(uri, ec);
    if (ec) {
        std::cout << "could not create connection because: " << ec.message() << std::endl;
        //TODO TBD: throw something
    } else {
        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        client_.connect(connection_);
    }
}

void Client::on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
    json jmessage = json::parse(msg->get_payload());
    if(jmessage["op"].get<int>() == 0){ //Dispatch
        std::map<std::string, std::function<void(json)>>::iterator it = eventResponses_.find(jmessage["t"]);
        if(it != eventResponses_.end()){
            asio_ios_.post(std::bind(it->second, jmessage));
        } else {
            std::cout << "There is no function for the event " << jmessage["t"] << ".\n";
        }
        if(jmessage["t"] == "READY") {
            uint32_t ms = jmessage["d"]["heartbeat_interval"];
            ms *= .9;
            keepalive(ms);
        }
    } else if(jmessage["op"].get<int>() == 1){ //Heartbeat (This isn't implemented yet, still using periodic heartbeats for now.)
        //client_.send(hdl, jmessage.dump(), websocketpp::frame::opcode::text);
    } else { //Wat
        std::cout << "Unexpected opcode received:\n\n" << jmessage.dump(4) << "\n\n\n";
    }
}

void Client::keepalive(uint32_t ms){
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    json message = {
            {"op", 1},
            {"d", millis}
    };
    std::cout << "Sending Heartbeat:\n";
    client_.send(connection_, message.dump(), websocketpp::frame::opcode::text);
    //reset timer
    keepalive_timer_.expires_from_now(std::chrono::milliseconds(ms));
    keepalive_timer_.async_wait(std::bind(&Client::keepalive, this, ms));
}

void Client::on_open(websocketpp::connection_hdl hdl){
    std::cout << "Connection established.\n";

    json connect = {
            {"op", 2},
            {"d", {
                           {"token", token_},
                           {"v", 4},
                           {"properties", {
                                                  {"$os", "linux"},
                                                  {"$browser", "discordpp"},
                                                  {"$device", "discordpp"},
                                                  {"$referrer",""}, {"$referring_domain",""}
                                          }
                           },
                           {"compress", false},
                           {"large_threshold", 250}
                   }
            }
    };
    std::cout << "Client Handshake:\n" << connect.dump(1) << "\n";
    client_.send(hdl, connect.dump(), websocketpp::frame::opcode::text);
}
std::string Client::fetchGateway(std::string token){
    return DiscordAPI::call("/gateway", token).at("url");
}*/