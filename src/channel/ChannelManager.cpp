#include "channel/ChannelManager.hpp"
#include "channel/MessageHeader.hpp"
#include "connection/Connection.hpp"
#include "utils/Defines.hpp"
#include "utils/Logging.hpp"

namespace nsf
{

ChannelManager::ChannelManager(const Config& _config, const sf::Clock& _systemClock)
    : m_systemClock{_systemClock}
    , m_timeAfterResendReliableMessageMs{_config.timeAfterResendReliableMessageMs}
{
    if (m_timeAfterResendReliableMessageMs == 0)
        m_timeAfterResendReliableMessageMs = DEFAULT_TIME_AFTER_RESEND_RELIABLE_MESSAGE_ms;
}

void ChannelManager::init(ChannelManagerCallbacks _callbacks)
{
    m_callbacks = _callbacks;
}

void ChannelManager::send(NetworkMessage&& _message)
{
    if (_message.isBroadcast())
    {
        for (Peer& peer : m_peers)
            peer.send(_message);
    }
    else if(_message.isExcludeBroadcast())
    {
        for (Peer& peer : m_peers)
        {
            if (peer.getPeerId() == _message.getPeerId())
                continue;
            peer.send(_message);
        }
    }
    else
    {
        Peer* peer = getPeer(_message.getPeerId());
        if (peer)
            peer->send(_message);
        else
            NSF_LOG("Don't send the message to a non-existent peer " << _message.getPeerId());
    }
}

void ChannelManager::deliverMessages()
{
    auto deliverFunc = [this](std::queue<NetworkMessage>& _messages){
        while(!_messages.empty())
        {
            auto& mes = _messages.front();
            m_callbacks.onReceive(std::move(mes));
            _messages.pop();
        }
    };
    for (Peer& peer : m_peers)
    {
        deliverFunc(peer.m_reliableMessagesToDeliver);
        deliverFunc(peer.m_unreliableMessagesToDeliver);

        peer.m_hasReliableDataToSendInTheCurrentFrame = !peer.m_reliableMessagesToSend.empty();
    }
}

void ChannelManager::onConnected(Connection& _connection)
{
    if (getPeer(_connection.getConnectionId()))
    {
        NSF_LOG_ERROR("Peer already exists " << _connection.getConnectionId());
        return;
    }

    Peer peer;
    peer.m_peerId = _connection.getConnectionId();
    m_peers.push_back(peer);
}

void ChannelManager::onDisconnected(Connection& _connection)
{
    if (!getPeer(_connection.getConnectionId()))
    {
        NSF_LOG_ERROR("Peer doesn't exist " << _connection.getConnectionId());
        return;
    }

    m_peers.erase(std::remove_if(m_peers.begin(), m_peers.end(), 
    [&_connection](const Peer& _peer) {
            return _peer.m_peerId == _connection.getConnectionId();
    }), m_peers.end());
}

bool ChannelManager::hasDataToWrite(ConnectionID _connectionId) const
{
    const Peer* peer = getPeer(_connectionId);
    if(!peer)
        return false;
    return peer->hasDataToSend();
}

void ChannelManager::onWritePacket(ConnectionID _connectionId, SequenceNumber _packetSequenceNum, Buffer& _data)
{
    Peer* peer = getPeer(_connectionId);
    if(!peer)
        return;

    // TODO It would be nice to somehow prioritize messages

    unsigned currentSystemTimeMs = m_systemClock.getElapsedTime().asMilliseconds();

    size_t dataSizeUsed = _data.getDataSize();
    
    // Write reliable messages
    for (auto& message : peer->m_reliableMessagesToSend)
    {
        // do I need to sort the vector before sending (sort by time)

        // Check is the message fits into the packet.
        // TODO improve this, because when writing a new packet we will start iterating from the beginning.
        if (message.message.m_data.getDataSize() > MessageHeader::MaxMessageSizeBytes - dataSizeUsed)
            return;

        if (message.sentTimeMs > 0.f && currentSystemTimeMs - message.sentTimeMs < m_timeAfterResendReliableMessageMs)
            continue;

        MessageHeader header(message.sequenceNum, message.message.m_data.getDataSize(), Peer::RELIABLE_CHANNEL_ID);
        header.serialize(_data);

        _data.append(message.message.m_data.getData(), message.message.m_data.getDataSize());
        message.sentTimeMs = currentSystemTimeMs;
        message.packetSequenceNum = _packetSequenceNum;

        dataSizeUsed = _data.getDataSize();

        NSF_LOG_DEBUG("ChannelManager::Write reliable message: " << message.sequenceNum << " in packet: " << _packetSequenceNum);
    }
    peer->m_hasReliableDataToSendInTheCurrentFrame = false;

    // Write unreliable messages
    while (!peer->m_unreliableMessagesToSend.empty())
    {
        NetworkMessage& message = peer->m_unreliableMessagesToSend.front();

        if (message.m_data.getDataSize() > MessageHeader::MaxMessageSizeBytes - dataSizeUsed)
            return;

        MessageHeader header(0, message.m_data.getDataSize(), Peer::UNRELIABLE_CHANNEL_ID);
        header.serialize(_data);

        _data.append(message.m_data.getData(), message.m_data.getDataSize());
        peer->m_unreliableMessagesToSend.pop();

        dataSizeUsed = _data.getDataSize();
    }    
}

void ChannelManager::onReadPacket(ConnectionID _connectionId, SequenceNumber _packetSequenceNum, Buffer& _data)
{
    NSF_UNUSED(_packetSequenceNum);

    Peer* peer = getPeer(_connectionId);
    if(!peer)
        return;

    size_t readPos = PacketHeader::PacketHeaderSizeBytes;   // It should be getReadPos

    const size_t dataSize = _data.getDataSize();
    while (readPos < dataSize)
    {
        // TODO to redo this part 

        Buffer localBuffer;
        localBuffer.append(reinterpret_cast<const std::byte*>(_data.getData()) + readPos, MessageHeader::MessageHeaderSizeBytes);

        MessageHeader header;
        header.deserialize(localBuffer);
        readPos += MessageHeader::MessageHeaderSizeBytes;

        NetworkMessage message;
        message.m_data.append(reinterpret_cast<const std::byte*>(_data.getData()) + readPos, header.messageSize);
        readPos += header.messageSize;

        message.m_info.m_peerId = peer->getPeerId();
        peer->onMessageReceived(header, std::move(message));
    } 
}

void ChannelManager::onPacketAcked(ConnectionID _connectionId, const std::unordered_set<SequenceNumber>& _ackedSequenceArray)
{
    Peer* peer = getPeer(_connectionId);
    if(!peer)
        return;
    
    NSF_LOG_DEBUG("ChannelManager::onPacketAcked, connection: " << _connectionId << " size: " << _ackedSequenceArray.size());
    
    if (peer->m_reliableMessagesToSend.empty())
        return;

    peer->m_reliableMessagesToSend.erase(
        std::remove_if(peer->m_reliableMessagesToSend.begin(), peer->m_reliableMessagesToSend.end(),
        [&_ackedSequenceArray](auto& _message){
            return _ackedSequenceArray.find(_message.packetSequenceNum) != _ackedSequenceArray.end();
        }),
        peer->m_reliableMessagesToSend.end()
    );   
}

Peer* ChannelManager::getPeer(PeerID _peerId)
{
    for (Peer& peer : m_peers)
        if (peer.m_peerId == _peerId)
            return &peer;
    return nullptr;
}

const Peer* ChannelManager::getPeer(PeerID _peerId) const
{
    for (const Peer& peer : m_peers)
        if (peer.m_peerId == _peerId)
            return &peer;
    return nullptr;
}

} // namespace nsf
