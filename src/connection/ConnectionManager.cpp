#include "connection/ConnectionManager.hpp"
#include "connection/InternalPacketType.hpp"
#include "connection/PacketHeader.hpp"

#include "utils/Assert.hpp"
#include "utils/Defines.hpp"
#include "utils/Logging.hpp"

#include "Constants.hpp"

#include <SFML/Network.hpp>

namespace nsf
{

ConnectionManager::ConnectionManager(const Config& _config)
    : UdpSocket(_config)
    , m_isServer{_config.isServer}
{
}

void ConnectionManager::init(ConnectionManagerCallbacks _callbacks)
{
    m_callbacks = _callbacks;
}

void ConnectionManager::connect(NetworkAddress _hostAddress)
{
    if (m_isServer)
    {
        NSF_LOG_ERROR("ConnectionManager::Cannot request connections as a server!");
        // TODO onConnectFailed
        return;
    }

    if (getConnection(_hostAddress))
    {
        NSF_LOG_ERROR("ConnectionManager::Already has a connection to " << _hostAddress.toString());
        return;
    }
    // what if it is disconnecting
    Connection connection;
    connection.m_status = Connection::Status::REQUESTING_CONNECTION;
    connection.m_address = _hostAddress;
   // connection.m_timeout = TIME_TO_RETRY_CONNECT_s;
    connection.m_connectionAttemptsLeft = 5;
    connection.m_connectionId = ++m_connectionIdGenerator;
    m_connections.push_back(connection);
}

void ConnectionManager::disconnect()
{
    // TODO
    NSF_ASSERT(false, "To implement");
}

void ConnectionManager::receive(sf::Time _dt)
{
    UdpSocket::receive(); // receive
    updateConnections(_dt);

    // UdpSocket send
}

void ConnectionManager::send(ConnectionID _connectionId, Buffer& _data)
{
    if (auto* connection = getConnection(_connectionId))
    {
        UdpSocket::send(_data, connection->getAddress());
    }
    else
    {
        NSF_LOG_ERROR("ConnectionManager::There is no connection with such id: " << _connectionId);
    }
}

bool ConnectionManager::isConnectedOrConnecting(NetworkAddress _address) const
{
    if (getConnection(_address))
        return true;// TODO connection->m_status == Connection::Status::REQUESTING_CONNECTION || ;    
    return false;
}

Connection* ConnectionManager::getConnection(NetworkAddress _address)
{
    for (Connection& connection : m_connections)
        if (connection.getAddress() == _address)
            return &connection;
    return nullptr;
}

const Connection* ConnectionManager::getConnection(NetworkAddress _address) const
{
    for (const Connection& connection : m_connections)
        if (connection.getAddress() == _address)
            return &connection;
    return nullptr;
}

const Connection* ConnectionManager::getConnection(ConnectionID _connectionId) const
{
    for (const Connection& connection : m_connections)
        if (connection.getConnectionId() == _connectionId)
            return &connection;
    return nullptr;
}

void ConnectionManager::updateConnections(sf::Time _dt)
{
    for (auto& connection : m_connections)
    {
        switch (connection.m_status)
        {
        case Connection::Status::REQUESTING_CONNECTION:
        {
            processRequestingConnectionState(connection, _dt);
            break;
        }
        case Connection::Status::DECIDING_CONNECTION:
        {
            processDecidingConnectionState(connection, _dt);
            break;
        }
        case Connection::Status::CONNECTED:
        {
            processConnectedState(connection, _dt);
            break;
        }
        
        default:
            break;
        }
    }
}

void ConnectionManager::processRequestingConnectionState(Connection& _connection, sf::Time _dt)
{
    if (_connection.m_connectionAccepted)
    {
        _connection.m_status = Connection::Status::CONNECTED;
        onConnected(_connection);
        return;
    }
    _connection.m_timeout -= _dt.asSeconds();
    if (_connection.m_timeout > 0.f)
        return;

    --_connection.m_connectionAttemptsLeft;
    if (_connection.m_connectionAttemptsLeft <= 0)
    {
        NSF_LOG("ConnectionManager::No atempts left, disconnect peer " << _connection.m_address.toString());
        _connection.m_failReason = "No atempts left";
        _connection.m_status = Connection::Status::CONNECTION_FAILED;
        onConnectionFailed(_connection);
    }
    else
    {
        _connection.m_timeout = DEFAULT_TIME_AFTER_RETRY_CONNECT_s;

        sf::Packet packet;
        PacketHeader header(InternalPacketType::CONNECT_REQUEST); 
        NSF_LOG_DEBUG("ConnectionManager::Send a connect request to " << m_address.toString());  
        header.serialize(packet);
        UdpSocket::send(packet, _connection.m_address);
    }
}

void ConnectionManager::processDecidingConnectionState(Connection& _connection, sf::Time _dt)
{
    NSF_UNUSED(_dt);
     // Accept the connection right away
    NSF_LOG("ConnectionManager::Accept the connection " << _connection.m_address.toString());  

    sf::Packet packet;
    PacketHeader header(InternalPacketType::CONNECT_ACCEPT); 
    header.serialize(packet);
    UdpSocket::send(packet, _connection.m_address);

    _connection.m_status = Connection::Status::CONNECTED;
    onConnected(_connection);
}

void ConnectionManager::processConnectedState(Connection& _connection, sf::Time _dt)
{
    NSF_UNUSED(_connection); NSF_UNUSED(_dt);
}


void ConnectionManager::onReceivePacket(sf::Packet& _packet, NetworkAddress _senderAddress)
{
    Connection* senderConnection = getConnection(_senderAddress);
 
    PacketHeader header;
    header.deserialize(_packet);

    switch (header.type)
    {
    case InternalPacketType::CONNECT_REQUEST:
    {
        if (senderConnection)
        {
            NSF_LOG_ERROR("ConnectionManager::CONNECT_REQUEST received again from " << _senderAddress.toString());
            // send connect accepted
            // or check if it's not old 
            break;
        }
        if (!m_isServer)
        {
            NSF_LOG_ERROR("ConnectionManager::Cannot process CONNECT_REQUEST recieved from " << _senderAddress.toString() << " because not the host");
            break;
        }
        
        NSF_LOG("ConnectionManager::CONNECT_REQUEST received from " << _senderAddress.toString());

        createConnectionFromRequest(_senderAddress);
        
        break;
    }
    case InternalPacketType::CONNECT_ACCEPT:
    {
        if (!senderConnection)
        {
            NSF_LOG_ERROR("ConnectionManager::CONNECT_ACCEPT received from " << _senderAddress.toString() << " who we didn't ask");
            break;
        }
        else if (senderConnection->getStatus() != Connection::Status::REQUESTING_CONNECTION)
        {
            NSF_LOG_ERROR("ConnectionManager::The status of " << _senderAddress.toString() << " isn't REQUESTING_CONNECTION");
            break;
        }
        NSF_LOG("ConnectionManager::CONNECT_ACCEPT received from " << _senderAddress.toString());

        //senderConnection->onConnectionAccepted();
        senderConnection->m_connectionAccepted = true;

        // senderPeer->onConnectionAcceptReceived();
        // onConnect(*senderPeer);
        break;
    }
    case InternalPacketType::DISCONNECT:
    {
        if (!senderConnection)
        {
            NSF_LOG("ConnectionManager::DISCONNECT received from " << _senderAddress.toString() << " who is not in the list of peers");
            break;
        }
        else if (senderConnection->getStatus() == Connection::Status::DOWN)
        {
            NSF_LOG_ERROR("ConnectionManager::The status of " << _senderAddress.toString() << " is already DOWN");
            break;
        }

        NSF_LOG("ConnectionManager::DISCONNECT received from " << _senderAddress.toString());
        // senderPeer->close(true);
        break;
    }
    case InternalPacketType::HEARTBEAT:
    {
        // TODO
        // if (!senderConnection)
        // {
        //     NSF_LOG_ERROR("Received from " + _senderAddress.toString() + " who is not in the list of peers");
        //     break;
        // }
        // else if (senderConnection->getStatus() == Connection::Status::CONNECTING)
        // {
        //     senderConnection->onConnectionAcceptReceived();
        //     onConnect(*senderConnection);
        //     //NSF_LOG("Received a heartbeat from " + _senderAddress.toString() + " while waiting for connection accept");
        // }
        // else if (!senderConnection->isUp())
        // {
        //     //NSF_LOG_ERROR("Received from " + _senderAddress.toString() + " who is not UP");
        //     break;
        // }
        // senderPeer->onHeartbeatReceived();
        // //NSF_LOG_DEBUG("Received a heartbeat from " + _senderAddress.toString());
        break;
    }
    case InternalPacketType::USER_PACKET:
    {
        if (!senderConnection || senderConnection->getStatus() != Connection::Status::CONNECTED)
        {
            NSF_LOG("ConnectionManager::Received from " << _senderAddress.toString() << " who is not in the list of connections or not yet ready.");
            break;
        }
    //    NSF_LOG("Received from " << _senderAddress.toString());

        m_callbacks.onReceive(senderConnection->getConnectionId(), header, _packet);

        break;
    }
    default:
        break;
    }
}

Connection& ConnectionManager::createConnectionFromRequest(NetworkAddress _address)
{

    Connection newConnection;
    newConnection.m_address = _address;
    newConnection.m_status = Connection::Status::DECIDING_CONNECTION;
    newConnection.m_connectionId = ++m_connectionIdGenerator;
    m_connections.push_back(newConnection);
    
    return m_connections.back();
}

void ConnectionManager::onConnected(Connection& _connection)
{
    m_callbacks.onConnected(_connection);
}

void ConnectionManager::onConnectionFailed(Connection& _connection)
{
    NSF_UNUSED(_connection);
}

} // namespace nsf

