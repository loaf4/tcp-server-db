#include "server_impl.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"

#include <algorithm>
#include <boost/asio.hpp>
#include <cstddef>
#include <fstream>
#include <functional>
#include <istream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

/* session implementation */

std::regex const session::key_regex_("[A-Za-z0-9]+");
std::regex const session::txt_regex_("[A-Za-z0-9]+\\.txt");

void session::start() { do_read(); }

void session::do_read() {
    io::async_read_until(
       socket_,
       streambuf_,
       "\n",
       std::bind(&session::on_read, shared_from_this(), _1, _2)
   );
}

void session::on_read(error_code err, size_t bytes_transmitted) {
    if (!err) {
        std::string message;
        if (getline(std::istream(&streambuf_), message)) {
            message_handler(message);
            do_write();
            buffer_.clear();
        }
    } else {
        logger_->error("Error: {} on {}.{}", err.to_string(), socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

        socket_.close();
    }
}

void session::do_write() {
    io::async_write(
       socket_,
       io::buffer(response_),
       std::bind(&session::on_write, shared_from_this(), _1, _2)
   );
}

void session::on_write(error_code err, size_t bytes_transmitted) {
    if (!err) {
        response_.clear();
        do_read();
    } else {
        logger_->error("Error: {} on {}.{}", err.to_string(), socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

        socket_.close();
    }
}

void session::message_handler(std::string const& message) {
    logger_->info("Request \"{}\" received from {}.{}", message, socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
    logger_->flush();
    if (message.starts_with("PUT ")) {
        on_put(message);
    } else if (message.starts_with("GET ")) {
        on_get(message);
    } else if (message.starts_with("DEL ")) {
        on_del(message);
    } else if (message == "COUNT") {
        on_count(message);
    } else if (message.starts_with("DUMP ")) {
        on_dump(message);
    } else {
        logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

        response_ = "NE\n";
    }
}

void session::on_put(std::string const& message) {
    std::istringstream ss(message);
    std::string tmp;
    std::string key;
    int cnt {};

    ss >> tmp;
    while (ss >> tmp) {
        if (++cnt == 3) {
            logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
            logger_->flush();

            response_ = "NE\n";
            return;
        }
        if (cnt == 1) {
            if (!is_valid_key(tmp)) {
                logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
                logger_->flush();

                response_ = "NE\n";
                return;
            }
            key = tmp;
        }
    }

    if (cnt != 2) {
        logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

        response_ = "NE\n";
        return;
    }

    if (std::string value = get_from_storage(key); value != "") {
        logger_->info("Response \"{}\" send to {}.{}", "OK " + value, socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

        response_ = "OK " + value + "\n";
    } else {
        logger_->info("Response \"{}\" send to {}.{}", "OK", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

        response_ = "OK\n";
    }

    put_in_storage(key, tmp);
}

void session::on_get(std::string const& message) {
    std::istringstream ss(message);
    std::string tmp;
    std::string value;
    int cnt {};

    ss >> tmp;
    while (ss >> tmp) {
        if (++cnt == 2) {
            logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
            logger_->flush();

            response_ = "NE\n";
            return;
        }
        if (!is_valid_key(tmp)) {
            logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
            logger_->flush();

            response_ = "NE\n";
            return;
        }
        value = get_from_storage(tmp);
    }

    if (cnt != 1 || value == "") {
        logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

        response_ = "NE\n";
        return;
    }

    logger_->info("Response \"{}\" send to {}.{}", "OK " + value, socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
    logger_->flush();

    response_ = "OK " + value + "\n";
}

void session::on_del(std::string const& message) {
    std::istringstream ss(message);
    std::string tmp;
    std::string value;
    int cnt {};

    ss >> tmp;
    while (ss >> tmp) {
        if (++cnt == 2) {
            logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
            logger_->flush();

            response_ = "NE\n";
            return;
        }
        if (!is_valid_key(tmp)) {
            logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
            logger_->flush();

            response_ = "NE\n";
            return;
        }
        value = get_from_storage(tmp);
    }

    if (cnt != 1 || value == "") {
        logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

        response_ = "NE\n";
        return;
    }

    storage_.erase(tmp);
    logger_->info("Response \"{}\" send to {}.{}", "OK " + value, socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
    logger_->flush();

    response_ = "OK " + value + "\n";
}

void session::on_count(std::string const& message) {
    size_t cnt{storage_.size()};

    logger_->info("Response \"{}\" send to {}.{}", "OK " + std::to_string(cnt), socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
    logger_->flush();

    response_ = "OK " + std::to_string(cnt) + "\n";
}

void session::on_dump(std::string const& message) {
    std::istringstream ss(message);
    std::string tmp;
    int cnt{};

    ss >> tmp;
    while (ss >> tmp) {
        if (++cnt == 2) {
            logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
            logger_->flush();

            response_ = "NE\n";
            return;
        }
    }

    if (cnt != 1 || !is_valid_txt(tmp)) {
        logger_->info("Response \"{}\" send to {}.{}", "NE", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
        logger_->flush();

        response_ = "NE\n";
        return;
    }

    // dumping
    logger_->info("Response \"{}\" send to {}.{}", "OK", socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());
    logger_->flush();

    server_.dump_storage(tmp);
    response_ = "OK\n";
}

bool session::is_valid_key(std::string const& key) {
    return std::regex_match(key, key_regex_); 
}

bool session::is_valid_txt(std::string const& fname) { 
    return std::regex_match(fname, txt_regex_); 
}

std::string session::get_from_storage(std::string const& key) {
    if (auto it = storage_.find(key); it != storage_.end()) {
        return it->second;
    }
    return "";
}

void session::put_in_storage(std::string const& key, std::string const& value) { storage_[key] = value; }

/* server implemetation */

std::string server::current_time_and_date() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d__%H-%M-%S");
    return ss.str();
}

void server::dump_storage(std::string const &filename) {
    std::ofstream out(dump_dir_ / ("dump_" + filename));

    for (auto const &it : storage_) {
        out << it.first << ":" << it.second << "\n";
    }

    out.close();
}

void server::upload_storage(std::string const &filename) {
    std::ifstream in(filename);

    std::string tmp;
    std::string::size_type pos;
    while(std::getline(in, tmp)) {
        pos = tmp.find(":");
        storage_[tmp.substr(0, pos)] = tmp.substr(pos + 1);
    }
}

void server::async_accept() {
    acceptor_.async_accept([this](error_code err, tcp::socket socket) {
        if (!err) {
            std::make_shared<session>(
                *this,
                std::move(socket),
                storage_,
                logger_
            )->start();

            async_accept();
        } else {
            logger_->error("Error: {} on {}.{}", err.to_string(), socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());
            logger_->flush();
        }
    });
}
