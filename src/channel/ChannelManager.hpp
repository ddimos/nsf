#pragma once
#include "channel/Peer.hpp"
#include "connection/PacketHeader.hpp"
#include "InternalTypes.hpp"

#include <functional>
#include <vector>
#include <unordered_set>

namespace nsf
{

class Connection;

struct ChannelManagerCallbacks
{
   // std::function<void(ConnectionID, Buffer&)> onSend{};
    std::function<void(NetworkMessage&& _message)> onReceive{};

};

class ChannelManager
{
public:
    ChannelManager(const Config& _config, const sf::Clock& _systemClock);
    ~ChannelManager() = default;

    void init(ChannelManagerCallbacks _callbacks);

    void send(NetworkMessage&& _message);
    void deliverMessages();

    void onConnected(Connection& _connection);
    void onDisconnected(Connection& _connection);

    bool hasDataToWrite(ConnectionID _connectionId) const;

    void onWritePacket(ConnectionID _connectionId, SequenceNumber _sequenceNum, Buffer& _data);
    void onReadPacket(ConnectionID _connectionId, SequenceNumber _sequenceNum, Buffer& _data);
    void onPacketAcked(ConnectionID _connectionId, const std::unordered_set<SequenceNumber>& _ackedSequenceArray);

private:
    Peer* getPeer(PeerID _peerId);
    const Peer* getPeer(PeerID _peerId) const;

    const sf::Clock& m_systemClock;
    ChannelManagerCallbacks m_callbacks{};
    std::vector<Peer> m_peers;

    unsigned m_timeAfterResendReliableMessageMs = 0u;
};

} // namespace nsf
