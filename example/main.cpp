#include <iostream>

#include <nsf/Network.hpp>

int main()
{
    unsigned port = 20475;
    bool result = nsf::Network::StartUp(port);

    if (!result)
    {
        std::cout << "Unable to create Network!" << std::endl;
        return 0;
    }
    
    std::cout << "Network is created!" << std::endl;
    std::cout << "The public address : " << nsf::Network::Get().GetPublicAddress().toString() << std::endl;
    std::cout << "The local address : " << nsf::Network::Get().GetLocalAddress().toString() << std::endl;

    nsf::Network::ShutDown();
}