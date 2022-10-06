#include "server.h"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "[*]Usage: ./server <port>\n";
        return 1;
    }

    uint16_t port = 0;
    try {
        port = uint16_t(std::stoi(argv[1]));
    } catch (...) {
        std::cout << "[-]Incorrect port number:" << port << "\n";
        return 1;
    }

    SMServer server(port);
    server.start();
    server.update();

    std::cout << "END\n";
    return 0;
}
