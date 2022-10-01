#include "server.h"

int main()
{
    SMServer server(5000);
    server.start();
    server.update();

    std::cout << "END\n";
    return 0;
}
