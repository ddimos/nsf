#include "Connection.hpp"
// #include "Utils/Log.h"
#include "InternalPacketType.hpp"
#include "PacketHeader.hpp"

namespace nsf
{

Connection::Connection(Transport& _transport, NetworkAddress _addressToConnect, bool _isCreatingFromRequest)
    : m_address(_addressToConnect)
    , m_transport(&_transport)
{
    sf::Packet packet;
    PacketHeader header;
    if (!_isCreatingFromRequest)
    {
        header.type = InternalPacketType::INTERNAL_CONNECT_REQUEST;
        m_status = Status::CONNECTING;  
        m_timeout = TIME_TO_RETRY_CONNECT_s;
    }
    else
    {
        header.type = InternalPacketType::INTERNAL_CONNECT_ACCEPT;
        m_status = Status::UP;
        m_timeout = HEARTBEAT_TIMEOUT_s;
    }
    header.serialize(packet);
    send(packet, _addressToConnect);
    //LOG_DEBUG("Send a connect request to " + m_address.toString());
}

void Connection::close(bool _forcibly /*= false*/)
{
    m_status = Status::DOWN;  
    if (!_forcibly)
    {
        // TODO: wwait for the response or AR
        sf::Packet packet;
        PacketHeader header;
        header.type = InternalPacketType::INTERNAL_DISCONNECT;
        header.serialize(packet);
        send(packet, m_address);
    } 
    //LOG_DEBUG("Disconnect from " + m_address.toString() + (_forcibly?" f":" g"));
}

void Connection::update(float _dt)
{
    m_timeout -= _dt;
    if (m_status == Status::CONNECTING)
    {
        if (m_timeout <= 0.f)
        {
            --m_connectionAttemptsLeft;
            if (m_connectionAttemptsLeft < 0)
            {
                //LOG("No atempts left, disconnect peer " + m_address.toString());
                m_status = Status::DOWN;
            }
            else
            {
                sf::Packet packet;
                PacketHeader header;
                header.type = InternalPacketType::INTERNAL_CONNECT_REQUEST; 
                m_timeout = TIME_TO_RETRY_CONNECT_s;
                //LOG_DEBUG("Send a connect request to " + m_address.toString());  
                header.serialize(packet);
                send(packet, m_address);
            }
        }
    }
    else if (m_status == Status::UP)
    {
        if (m_timeout <= 0.f)
        {       
            //LOG("Disconnect peer " + m_address.toString() + " because didn't receive a heartbeat");
            m_status = Status::DOWN;
        }
        else
        {
            m_heartbeat -= _dt;
            if (m_heartbeat <= 0)
            {
                sf::Packet packet;
                PacketHeader header;
                header.type = InternalPacketType::INTERNAL_HEARTBEAT; 
                //LOG_DEBUG("Send a heartbeat to " + m_address.toString()); 
                header.serialize(packet);
                send(packet, m_address);   
                m_heartbeat = HEARTBEAT_s;
            }
        }
    }
}

void Connection::send(sf::Packet _packet, NetworkAddress _address)
{
    m_transport->send(_packet, _address);
}

void Connection::onConnectionAcceptReceived()
{
    m_status = Status::UP;
    m_timeout = HEARTBEAT_TIMEOUT_s;
}

void Connection::onHeartbeatReceived()
{
    m_timeout = HEARTBEAT_TIMEOUT_s;
}

} // namespace nsf
