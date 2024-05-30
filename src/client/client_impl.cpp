#include <iostream>

#include "client_impl.h"

void client::start(tcp::endpoint ep) {
    socket_.async_connect(
        ep,
        // std::bind(&client::on_connect, shared_from_this(), _1)
        [this](error_code err) {
            on_connect(err);
        }
    );
}

void client::on_connect(error_code const &err) {
    if (!err) {
        async_write();
    } else {
        socket_.close();
    }
}

void client::on_read(error_code const &err, std::size_t bytes_transmitted) {
    if (!err) {
        std::stringstream message(response_);
        message << std::istream(&streambuf_).rdbuf();
        streambuf_.consume(bytes_transmitted);

        std::cout << response_ << std::endl;
        response_.clear();

        async_write();
    } else {
        socket_.close();
    }
}

void client::async_read() {
    auto self = shared_from_this();
    io::async_read_until(
        socket_,
        streambuf_,
        "\n",
        // std::bind(&client::on_read, shared_from_this(), _1, _2)
        [self](error_code err, std::size_t bytes_transmitted) {
            self->on_read(err, bytes_transmitted);
        }
    );
}

void client::on_write(error_code const &err) {
    if (!err) {
        async_read();
    } else {
        socket_.close();
    }
}

void client::async_write() {
    std::getline(std::cin, request_);
    request_ += "\n";
    io::async_write(
        socket_,
        io::buffer(request_), 
        // std::bind(&client::on_write, shared_from_this(), _1)
        [this](error_code err, std::size_t bytes_transmitted) {
            on_write(err);
        }
    );
}

