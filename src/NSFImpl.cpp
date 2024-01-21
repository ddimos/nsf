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
    : m_connectionManager(_config)
    , m_packetManager{m_systemClock}
    , m_channelManager{_config, m_systemClock}
    , m_config{_config}
    , m_callbacks{_callbacks}
{
    m_systemClock.restart();

    ConnectionManagerCallbacks cmCallbacks;
    cmCallbacks.onConnected = 
        [this](Connection& _connection) {
            m_packetManager.onConnected(_connection);
            m_channelManager.onConnected(_connection);

            m_callbacks.onConnected(_connection.getConnectionId());
        };
    cmCallbacks.onDisconnected = 
        [this](Connection& _connection) {
            NSF_LOG("TODO disconnect");
            m_callbacks.onDisconnected(_connection.getConnectionId());
        };
    cmCallbacks.onReceive = 
        [this](ConnectionID _connectionId, PacketHeader _header, Buffer& _buffer) {
            m_packetManager.receive(_connectionId, _header, _buffer);
        };
    m_connectionManager.init(cmCallbacks);

    PacketManagerCallbacks pmCallbacks;
    pmCallbacks.onSend = 
        [this](ConnectionID _connectionId, Buffer& _data) {
            m_connectionManager.send(_connectionId, _data);
        };

    pmCallbacks.haveDataToSend =
        [this](ConnectionID _connectionId) {
            return m_channelManager.hasDataToWrite(_connectionId);
        };
    pmCallbacks.onWritePacket =
        [this](ConnectionID _connectionId, SequenceNumber _sequenceNumber, Buffer& _data) {
            m_channelManager.onWritePacket(_connectionId, _sequenceNumber, _data);
        };
    pmCallbacks.onReadPacket =
        [this](ConnectionID _connectionId, SequenceNumber _sequenceNumber, Buffer& _data) {
            m_channelManager.onReadPacket(_connectionId, _sequenceNumber, _data);
        };
    pmCallbacks.onPacketAcked =
        [this](ConnectionID _connectionId, const std::unordered_set<SequenceNumber>& _ackedSequenceArray) {
            m_channelManager.onPacketAcked(_connectionId, _ackedSequenceArray);
        };

    m_packetManager.init(pmCallbacks);  // TODO to redo the init (not to forget anything)

    ChannelManagerCallbacks chanManCallbacks;

    chanManCallbacks.onReceive =
        [this](NetworkMessage&& _message) {
            m_callbacks.onReceived(std::move(_message));
        };
    m_channelManager.init(chanManCallbacks);
}

void NSFImpl::updateReceive()
{
    sf::Time elapsed = m_deltaClock.restart();
    m_connectionManager.receive(elapsed);

    m_channelManager.deliverMessages();
}

void NSFImpl::updateSend()
{
    m_packetManager.sendAll();
}

void NSFImpl::send(NetworkMessage&& _message)
{
    m_channelManager.send(std::move(_message));
}

void NSFImpl::connect(NetworkAddress _address)
{
    m_connectionManager.connect(_address);
}

void NSFImpl::disconnect()
{
    m_connectionManager.disconnect();
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
    return m_connectionManager.getPublicAddress();
}

NetworkAddress NSFImpl::getLocalAddress() const
{
    return m_connectionManager.getLocalAddress();
}

} // namespace nsf
