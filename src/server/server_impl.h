#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <regex>
#include "spdlog/common.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"


namespace fs = std::filesystem;
namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;
using namespace std::placeholders;

class server;

class session : public std::enable_shared_from_this<session> {

public:
    session(server &server, tcp::socket socket, std::unordered_map<std::string, std::string> &storage, std::shared_ptr<spdlog::logger> logger) :
        server_ {server},
        socket_(std::move(socket)),
        storage_ {storage},
        logger_ {logger} {

        logger_->info("{}.{} connected to server", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

    }

    void start();

private:

    void do_read();
    void on_read(error_code err, std::size_t bytes_transmitted);

    void do_write();
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

    server &server_;
    tcp::socket socket_;
    io::streambuf streambuf_;
    std::string buffer_;
    std::string response_;

    std::unordered_map<std::string, std::string> &storage_;
    std::shared_ptr<spdlog::logger> logger_;
    const static std::regex key_regex_;
    const static std::regex txt_regex_;

};

class server {

public:

    server(io::io_context& io_context, std::uint16_t port, fs::path log_dir, fs::path dump_dir, std::string const &upload_path = "") :
        io_context_(io_context),
        acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)),
        log_dir_ {log_dir}, 
        dump_dir_ {dump_dir} {

        try {
            logger_ = spdlog::basic_logger_mt("basic", (log_dir_ / (current_time_and_date() + ".txt")).c_str());
        } catch (const spdlog::spdlog_ex &ex) {
            std::cout << "Log init failed: " << ex.what() << std::endl;
            exit(1);
        }

        logger_->info("Server starts on {} port", port);
        logger_->flush();

        if (upload_path != "") {
            logger_->info("Uploads dump from {}", upload_path);
            logger_->flush();

            upload_storage(upload_path);
        }

        async_accept();
    }

    void async_accept();

    void dump_storage(std::string const &filename);
    void upload_storage(std::string const &filename);

    std::string current_time_and_date();

  private:

    io::io_context& io_context_;
    tcp::acceptor acceptor_;

    std::unordered_map<std::string, std::string> storage_;

    std::shared_ptr<spdlog::logger> logger_;
    fs::path log_dir_;
    fs::path dump_dir_;

};

#endif
