#include <iostream>
#include <memory>
#include <regex>
#include <string>

#include "client_impl.h"

namespace io = boost::asio;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "usage: client <ip> <port>" << std::endl;
        return 1;
    }

    // std::regex addr_regex("((2([0-4][0-9]|5[0-5])?|[0-1]?[0-9]?[0-9])?\\.){3}(2([0-4][0-9]|5[0-5])?|[0-1]?[0-9]?[0-9])");
    // if (std::regex_match(argv[1], addr_regex)) {
    //     std::cout << "invalid ip address" << std::endl;
    //     return 1;
    // }

    io::io_context io_context;
    std::make_shared<client>(io_context, "127.0.0.1", static_cast<uint16_t>(std::stoi(argv[2])));
    io_context.run();

    return 0;
}
