#pragma once
#include "nsf/NetworkAddress.hpp"
#include "nsf/Types.hpp"

#include "connection/Connection.hpp"
#include "connection/PacketHeader.hpp"
#include "connection/UdpSocket.hpp"

#include <functional>
#include <vector>

namespace nsf
{

struct ConnectionManagerCallbacks
{
    std::function<void(Connection&)> onConnected{};
    std::function<void(Connection&)> onDisconnected{};
    // TODO to add onConnectFailed
    
    std::function<void(ConnectionID, PacketHeader, Buffer&)> onReceive{};
};

class ConnectionManager : public UdpSocket
{
public:
    ConnectionManager(const Config& _config);

    void init(ConnectionManagerCallbacks _callbacks);

    void connect(NetworkAddress _hostAddress);
    void disconnect();

    void receive(sf::Time _dt);
    void send(ConnectionID _connectionId, Buffer& _data);

private:
    bool isConnectedOrConnecting(NetworkAddress _address) const;

    Connection* getConnection(NetworkAddress _address);
    const Connection* getConnection(NetworkAddress _address) const;
    const Connection* getConnection(ConnectionID _connectionId) const;

    void updateConnections(sf::Time _dt);
    void processRequestingConnectionState(Connection& _connection, sf::Time _dt);
    void processDecidingConnectionState(Connection& _connection, sf::Time _dt);
    void processConnectedState(Connection& _connection, sf::Time _dt);

    void onReceivePacket(sf::Packet& _packet, NetworkAddress _sender) override;

    Connection& createConnectionFromRequest(NetworkAddress _address);

    void onConnected(Connection& _connection);
    void onConnectionFailed(Connection& _connection);


    const bool m_isServer = true;

    ConnectionID m_connectionIdGenerator = 0;
    std::vector<Connection> m_connections;

    ConnectionManagerCallbacks m_callbacks;
};

} // namespace nsf
