#include <iostream>

#include "client_impl.h"

void client::start(tcp::endpoint ep) {
    socket_.async_connect(
       ep,
       [this, self = shared_from_this()](error_code err) { on_connect(err); }
   );
}

void client::on_connect(error_code const& err) {
    if (!err) {
        do_write();
    } else {
        socket_.close();
    }
}

void client::on_read(error_code const& err, size_t bytes_transmitted) {
    if (!err) {
        std::cout << &streambuf_;
        response_.clear();
        do_write();
    } else {
        socket_.close();
    }
}

void client::do_read() {
    auto self = shared_from_this();
    io::async_read_until(
        socket_,
        streambuf_,
        "\n",
        [self](error_code err, size_t bytes_transmitted) {
            self->on_read(err, bytes_transmitted);
        }
    );
}

void client::on_write(error_code const& err) {
    if (!err) {
        do_read();
    } else {
        socket_.close();
    }
}

void client::do_write() {
    std::getline(std::cin, request_);
    request_ += "\n";
    io::async_write(
        socket_,
        io::buffer(request_),
        [this, self = shared_from_this()](error_code err, size_t bytes_transmitted) { on_write(err); });
}
