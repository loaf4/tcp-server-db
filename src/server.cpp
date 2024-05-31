#include <iostream>
#include <boost/asio.hpp>

#include "server_impl.h"

namespace io = boost::asio;

int main(int argc, char* argv[]) {

    if (argc < 3 || argc > 4) {
        std::cout << "usage: server <port> <limit_connections> [<dump_file>]" << std::endl;
        return 1;
    }

    io::io_context io_context;
    server srv(io_context, std::stoi(argv[1]));
    srv.async_accept();
    io_context.run();

    return 0;
}
