#pragma once
#include <SFML/Network.hpp>
#include "nsf/NetworkAddress.hpp"
#include <vector>

namespace nsf
{

class Transport
{
public:
    void send(sf::Packet _packet, NetworkAddress _address);
    NetworkAddress getLocalAddress() const { return m_localAddress; }
    NetworkAddress getPublicAddress() const { return m_publicAddress; }

protected:

    Transport(unsigned short _port);
    virtual ~Transport() = default;

    void update();

    virtual void onReceivePacket(sf::Packet _packet, NetworkAddress _sender) = 0;

private:
    void createHost(unsigned short _port);

    sf::UdpSocket m_localSocket;
    NetworkAddress m_localAddress = {};
    NetworkAddress m_publicAddress = {};
};

} // namespace nsf
