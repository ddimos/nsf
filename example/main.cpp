#include <iostream>

#include <nsf/NSF.hpp>

int main(int argc, char **argv)
{
    bool isHost = true;
    if (argc < 2)
    {
        std::cout << "No arguments passed, assume to be a host" << std::endl;
    }
    else
    {
        std::cout << "Create a client" << std::endl;
        isHost = false;
    }

    nsf::Config config;
    config.port = (isHost) ? 20475 : 20480;//20087
    auto network = nsf::createNSF(config);
    std::cout << "Network is created!" << std::endl;
    std::cout << "The public address : " << network->getPublicAddress().toString() << std::endl;
    std::cout << "The local address : "  << network->getLocalAddress().toString() << std::endl;

}