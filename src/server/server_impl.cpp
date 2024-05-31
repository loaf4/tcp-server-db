#include "server_impl.h"

#include <boost/asio.hpp>
#include <cstddef>
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
        socket_.close();
    }
}

void session::message_handler(std::string const& message) {
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
            response_ = "NE\n";
            return;
        }
        if (cnt == 1) {
            if (!is_valid_key(tmp)) {
                response_ = "NE\n";
                return;
            }
            key = tmp;
        }
    }

    if (cnt != 2) {
        response_ = "NE\n";
        return;
    }

    if (std::string value = get_from_storage(key); value != "") {
        response_ = "OK " + value + "\n";
    } else {
        response_ = "OK\n";
    }
    storage_[key] = tmp;
}

void session::on_get(std::string const& message) {
    std::istringstream ss(message);
    std::string tmp;
    std::string value;
    int cnt {};

    ss >> tmp;
    while (ss >> tmp) {
        if (++cnt == 2) {
            response_ = "NE\n";
            return;
        }
        if (!is_valid_key(tmp)) {
            response_ = "NE\n";
            return;
        }
        value = get_from_storage(tmp);
    }

    if (cnt != 1 || value == "") {
        response_ = "NE\n";
        return;
    }

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
            response_ = "NE\n";
            return;
        }
        if (!is_valid_key(tmp)) {
            response_ = "NE\n";
            return;
        }
        value = get_from_storage(tmp);
    }

    if (cnt != 1 || value == "") {
        response_ = "NE\n";
        return;
    }

    storage_.erase(tmp);
    response_ = "OK " + value + "\n";
}

void session::on_count(std::string const& message) {
    size_t cnt{storage_.size()};
    response_ = "OK " + std::to_string(cnt) + "\n";
}

void session::on_dump(std::string const& message) {
    std::istringstream ss(message);
    std::string tmp;
    int cnt{};

    ss >> tmp;
    while (ss >> tmp) {
        if (++cnt == 2) {
            response_ = "NE\n";
            return;
        }
    }

    if (cnt != 1 || !is_valid_txt(tmp)) {
        response_ = "NE\n";
        return;
    }

    // dumping
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

void session::put_from_storage(std::string const& key, std::string const& value) { storage_[key] = value; }

/* server implemetation */

void server::async_accept() {
    acceptor_.async_accept([this](error_code err, tcp::socket socket) {
        if (!err) {
            std::make_shared<session>(std::move(socket), storage_)->start();
            async_accept();
        }
    });
}
