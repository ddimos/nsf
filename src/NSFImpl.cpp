#include "NSFImpl.hpp"

#include "connection/InternalPacketType.hpp"
#include "connection/PacketHeader.hpp"

#include "utils/Assert.hpp"
#include "utils/Logging.hpp"

namespace nsf
{

std::unique_ptr<INSF> createNSF(const Config& _config, NSFCallbacks _callbacks)
{
    return std::make_unique<NSFImpl>(_config, _callbacks);
}

// ---------------------------------------------------------------

NSFImpl::NSFImpl(const Config& _config, NSFCallbacks _callbacks)
    : m_config{_config}
    , m_callbacks{_callbacks}
{
    ConnectionManagerCallbacks connectionManCallbacks;
    connectionManCallbacks.onConnected = 
        [this](Connection& _connection) {
            m_packetManager->onConnected(_connection);
            m_channelManager->onConnected(_connection);

            m_callbacks.onConnected(_connection.getConnectionId());
        };
    connectionManCallbacks.onDisconnected = 
        [this](Connection& _connection) {
            m_packetManager->onDisconnected(_connection);
            m_channelManager->onDisconnected(_connection);

            m_callbacks.onDisconnected(_connection.getConnectionId());
        };
    connectionManCallbacks.onReceivePacket = 
        [this](ConnectionID _connectionId, PacketHeader _header) {
            m_packetManager->onReceivePacket(_connectionId, _header);
        };
    connectionManCallbacks.onSendPacket = 
        [this](ConnectionID _connectionId) {
            return m_packetManager->onSendPacket(_connectionId);
        };
    connectionManCallbacks.haveDataToSend = 
        [this](ConnectionID _connectionId) {
            return m_channelManager->hasDataToWrite(_connectionId);
        };
    connectionManCallbacks.onWritePacket = 
        [this](ConnectionID _connectionId, SequenceNumber _sequenceNumber, Buffer& _data) {
            m_channelManager->onWritePacket(_connectionId, _sequenceNumber, _data);
        };
    connectionManCallbacks.onReadPacket = 
        [this](ConnectionID _connectionId, SequenceNumber _sequenceNumber, Buffer& _data) {
            m_channelManager->onReadPacket(_connectionId, _sequenceNumber, _data);
        };

    PacketManagerCallbacks packetManCallbacks;
    packetManCallbacks.onPacketAcked =
        [this](ConnectionID _connectionId, const std::unordered_set<SequenceNumber>& _ackedSequenceArray) {
            m_channelManager->onPacketAcked(_connectionId, _ackedSequenceArray);
        };

    ChannelManagerCallbacks channelManCallbacks;
    channelManCallbacks.onReceive =
        [this](NetworkMessage&& _message) {
            m_callbacks.onReceived(std::move(_message));
        };

    m_connectionManager = std::make_unique<ConnectionManager>(_config, m_systemClock, connectionManCallbacks);
    m_packetManager = std::make_unique<PacketManager>(m_systemClock, packetManCallbacks);
    m_channelManager = std::make_unique<ChannelManager>(_config, m_systemClock, channelManCallbacks);

    m_systemClock.restart();
}

void NSFImpl::updateReceive()
{
    m_connectionManager->updateReceive();

    m_channelManager->updateReceive();
}

void NSFImpl::updateSend()
{
    m_connectionManager->updateSend();
}

void NSFImpl::send(NetworkMessage&& _message)
{
    m_channelManager->send(std::move(_message));
}

void NSFImpl::connect(NetworkAddress _address)
{
    m_connectionManager->connect(_address);
}

void NSFImpl::disconnect(PeerID _peerId/*= PEER_ID_INVALID*/)
{
    m_connectionManager->disconnect(_peerId);
}

bool NSFImpl::isServer() const
{
    return m_config.isServer;
}

PeerID NSFImpl::getServerPeerId() const
{
    NSF_ASSERT(false, "Implement");
    return PEER_ID_INVALID; // TODO implement
}

NetworkAddress NSFImpl::getPublicAddress() const
{
    return m_connectionManager->getPublicAddress();
}

NetworkAddress NSFImpl::getLocalAddress() const
{
    return m_connectionManager->getLocalAddress();
}

} // namespace nsf
