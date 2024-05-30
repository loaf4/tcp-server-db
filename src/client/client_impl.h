#ifndef CLIENT_IMPL_H
#define CLIENT_IMPL_H

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <cstddef>
#include <string>

namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;
using namespace std::placeholders;

class client : public boost::enable_shared_from_this<client> {

public:

    client(io::io_context &io_context, std::string const &addr, uint16_t port) 
    : socket_(io_context) {
        start(tcp::endpoint(
            io::ip::address::from_string(addr),
            port
        ));
    }

    void start(tcp::endpoint ep);

private:

    void on_connect(error_code const &err);

    void on_read(error_code const &err, std::size_t bytes_transmitted);
    void async_read();

    void on_write(error_code const &err);
    void async_write();

private:

    tcp::socket socket_;
    io::streambuf streambuf_;
    std::string request_;
    std::string response_;

};

#endif
