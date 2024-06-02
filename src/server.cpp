#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <boost/asio.hpp>
#include <string>

#include "server_impl.h"

namespace io = boost::asio;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {

    if (argc < 4 || argc > 5) {
        std::cout << "usage: server <port> <log_dir> <dump_dir> [<dump_file>]" << std::endl;
        return 1;
    }

    fs::path log_dir = argv[2];
    if (!fs::is_directory(log_dir)) { 
        std::cout << "Logging directory doesn't exist" << std::endl;
        exit(1);
    }
    fs::path dump_dir = argv[3];
    if (!fs::is_directory(dump_dir)) { 
        std::cout << "Dumping directory doesn't exist" << std::endl;
        exit(1);
    }
    std::string upload_dump {""};
    if (argc == 5 && fs::exists(argv[4])) {
        upload_dump = argv[4];
    }

    io::io_context io_context;
    server srv(io_context, std::stoi(argv[1]), log_dir, dump_dir, upload_dump);
    srv.async_accept();
    io_context.run();

    return 0;
}
