#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "usage: client <ip> <port>" << std::endl;
        return 1;
    }

    return 0;
}
