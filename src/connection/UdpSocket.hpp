#pragma once
#include "nsf/NetworkAddress.hpp"
#include <SFML/Network.hpp>

namespace nsf
{

struct Config;

class UdpSocket
{
public:
    NetworkAddress getLocalAddress() const { return m_localAddress; }
    NetworkAddress getPublicAddress() const { return m_publicAddress; }

protected:
    UdpSocket(const Config& _config);
    virtual ~UdpSocket() = default;

    void receive();
    void send(sf::Packet& _packet, NetworkAddress _address);

    virtual void onReceivePacket(sf::Packet& _packet, NetworkAddress _sender) = 0;

private:
    void createHost(unsigned short _port);

    sf::UdpSocket m_localSocket;
    NetworkAddress m_localAddress = {};
    NetworkAddress m_publicAddress = {};

    int m_packetDropChance = 0;
    int m_countToNextPacketDrop = 0;
};

} // namespace nsf
