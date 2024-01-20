#include "connection/UdpSocket.hpp"
#include "nsf/Types.hpp"
#include "utils/Logging.hpp"
#include "Constants.hpp"
#include <algorithm>

namespace nsf
{

UdpSocket::UdpSocket(const Config& _config)
    : m_packetDropChance{_config.packetDropChance}
    , m_countToNextPacketDrop{_config.packetDropChance}
{
    createHost(_config.port);
}

void UdpSocket::receive()
{
    while (true)
    {
        sf::Packet packet;
        NetworkAddress sender;
        const sf::Socket::Status status = m_localSocket.receive(packet, sender.address, sender.port);

        if (status == sf::Socket::Status::NotReady)
        {
            break;
        }
        else if(status == sf::Socket::Status::Done)
        {
            onReceivePacket(packet, sender);
        }
        else
        {
            NSF_LOG_ERROR("UdpSocket::The status of the socket: " << NSF_TO_STR(status));
            break;
        }       
    }

}

void UdpSocket::send(sf::Packet& _packet, NetworkAddress _address)
{
    if (m_packetDropChance != 0)
    {
        m_countToNextPacketDrop--;
        if (m_countToNextPacketDrop <= 0)
        {
            m_countToNextPacketDrop = m_packetDropChance;
            NSF_LOG("UdpSocket::Drop");
            return;
        }
    }

    m_localSocket.send(_packet, _address.address, _address.port);
}

void UdpSocket::createHost(unsigned short _port)
{
    int attemptsLeft = DEFAULT_BIND_PORT_ATTEMPTS_COUNT;
    while (m_localSocket.bind(_port) != sf::Socket::Done || attemptsLeft < 0)
    {
        ++_port;
        --attemptsLeft;
    }
    
    if (attemptsLeft < 0)
    {
        NSF_LOG_ERROR("UdpSocket::Couldn't bind a port");
        return;
    }

    m_localSocket.setBlocking(false);

    m_localAddress.address = sf::IpAddress::getLocalAddress();
    m_localAddress.port = m_localSocket.getLocalPort();
    m_publicAddress.address = sf::IpAddress::getPublicAddress();
    m_publicAddress.port = m_localSocket.getLocalPort();

    NSF_LOG_DEBUG("UdpSocket::Host created on a port:\t" << m_localSocket.getLocalPort()  
     << "\n\tLocal address:\t\t" << m_localAddress.address.toString()
     << "\n\tGlobal:\t\t\t" << m_publicAddress.address.toString());
}

} // namespace nsf
