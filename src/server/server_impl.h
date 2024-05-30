#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <regex>


namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;
using namespace std::placeholders;

// using message_handler = std::function<void(std::string)>;
// using error_handler = std::function<void()>;


class session : public std::enable_shared_from_this<session> {

public:
    session(tcp::socket socket, std::unordered_map<std::string, std::string> &storage) : 
        socket_(std::move(socket)),
        storage_ {storage} {}

    void start();

private:

    void async_read();
    void on_read(error_code err, std::size_t bytes_transmitted);

    void async_write();
    void on_write(error_code err, std::size_t bytes_transmitted);

    void message_handler(std::string const &message);
    void on_put(std::string const &message);
    void on_get(std::string const &message);
    void on_del(std::string const &message);
    void on_count(std::string const &message);
    void on_dump(std::string const &message);
    bool is_valid_key(std::string const &key);
    bool is_valid_txt(std::string const &fname);

    std::string get_from_storage(std::string const &key);
    void put_from_storage(std::string const &key, std::string const &value);

    tcp::socket socket_;
    io::streambuf streambuf_;
    std::string buffer_;
    std::string response_;

    std::unordered_map<std::string, std::string> &storage_;
    const static std::regex key_regex_;
    const static std::regex txt_regex_;

};

class server {

public:

    server(io::io_context& io_context, std::uint16_t port) :
        io_context_(io_context),
        acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)) {
        async_accept();
    }

    void async_accept();

  private:

    io::io_context& io_context_;
    tcp::acceptor acceptor_;

    std::unordered_map<std::string, std::string> storage_;

};

#endif
