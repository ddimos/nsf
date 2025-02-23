#pragma once
#include "connection/PacketHeader.hpp"
#include "InternalTypes.hpp"

#include <functional>
#include <tuple>
#include <unordered_set>
#include <vector>

// The acknowledgment algorithm is taken from 
// https://gafferongames.com/post/reliable_ordered_messages/

namespace nsf
{

class NetworkMessage;
class Connection;

struct PacketManagerCallbacks
{
    std::function<void(ConnectionID, const std::unordered_set<SequenceNumber>&)> onPacketAcked{};
};

class PacketManager
{
public:
    PacketManager(const sf::Clock& _systemClock, PacketManagerCallbacks _callbacks);
    ~PacketManager() = default;

    float getRtt(ConnectionID _connectionId) const;
    
    void onReceivePacket(ConnectionID _connectionId, PacketHeader _header);
    std::tuple<SequenceNumber, SequenceNumber, AckBits> onSendPacket(ConnectionID _connectionId);

    void onConnected(Connection& _connection);
    void onDisconnected(Connection& _connection);
private:
    
    struct PacketData
    {
        enum class State
        {
            EMPTY,
            ACKED,
            RECEIVED
        };

        bool isAcked() const { return state == State::ACKED; }
        bool isReceived() const { return state == State::RECEIVED; }

        float time = 0.f;  // sent/ received // perhaps only sent
        SequenceNumber sequence = 0;
        State state = State::EMPTY;
    };

    static constexpr int SequencePacketBufferSize{1024};    // TODO to move
    static constexpr int NumberOfAckBits{8*sizeof(AckBits)};
    
    struct PacketPeer
    {
        void onSendPacket(PacketData _data);
        void onReceivePacket(PacketData _data);

        AckBits generateAckBits();
        std::unordered_set<SequenceNumber> processAckBits(SequenceNumber _ack, AckBits _ackBits, float _time);

        ConnectionID m_connectionId = CONNECTION_ID_INVALID;
    
        SequenceNumber m_sequenceNumberGenerator = 0;
        
        SequenceNumber m_sentSequenceNumber = MAX_SEQUENCE_NUMBER;
        SequenceNumber m_receivedSequenceNumber = MAX_SEQUENCE_NUMBER;

        PacketData m_sentPacketData[SequencePacketBufferSize];
        PacketData m_receivedPacketData[SequencePacketBufferSize];

        float m_rtt = 0.f;
    };

    PacketPeer* getPeer(ConnectionID _connectionId);

    const sf::Clock& m_systemClock;

    PacketManagerCallbacks m_callbacks;
    std::vector<PacketPeer> m_peers;
};

} // namespace nsf
