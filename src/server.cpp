#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 5) {
        std::cout << "usage: server <ip> <port> <limit_connections> [<dump_file>]" << std::endl;
        return 1;
    }

    return 0;
}
