#pragma once
#include "nsf/NetworkAddress.hpp"
#include "nsf/Types.hpp"

#include "connection/Connection.hpp"
#include "connection/PacketHeader.hpp"
#include "connection/UdpSocket.hpp"

#include <functional>
#include <tuple>
#include <vector>

namespace nsf
{

struct ConnectionManagerCallbacks
{
    std::function<void(Connection&)> onConnected{};
    std::function<void(Connection&)> onDisconnected{};
    // TODO to add onConnectFailed
    
    std::function<void(ConnectionID, PacketHeader)> onReceivePacket{};
    std::function<std::tuple<SequenceNumber, SequenceNumber, AckBits>(ConnectionID)> onSendPacket{};

    std::function<bool(ConnectionID)> haveDataToSend{};
    std::function<void(ConnectionID, SequenceNumber, Buffer&)> onWritePacket{};
    std::function<void(ConnectionID, SequenceNumber, Buffer&)> onReadPacket{};

};

class ConnectionManager : public UdpSocket
{
public:
    ConnectionManager(const Config& _config, const sf::Clock& _systemClock, ConnectionManagerCallbacks _callbacks);

    void connect(NetworkAddress _hostAddress);
    void disconnect();

    void updateReceive();
    void updateSend();

private:
    bool isConnectedOrConnecting(NetworkAddress _address) const;

    Connection* getConnection(NetworkAddress _address);
    const Connection* getConnection(NetworkAddress _address) const;
    const Connection* getConnection(ConnectionID _connectionId) const;

    void updateConnections(sf::Time _systemTime);
    void processRequestingConnectionState(Connection& _connection, sf::Time _systemTime);
    void processDecidingConnectionState(Connection& _connection, sf::Time _systemTime);
    void processConnectedState(Connection& _connection, sf::Time _systemTime);

    void onReceivePacket(sf::Packet& _packet, NetworkAddress _sender) override;

    Connection& createConnectionFromRequest(NetworkAddress _address);

    void onConnected(Connection& _connection);
    void onConnectionFailed(Connection& _connection);

    const bool m_isServer = true;
    const sf::Clock& m_systemClock;

    ConnectionManagerCallbacks m_callbacks;

    ConnectionID m_connectionIdGenerator = 0;
    std::vector<Connection> m_connections;
};

} // namespace nsf
