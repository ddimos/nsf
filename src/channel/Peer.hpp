#pragma once
#include "nsf/NetworkAddress.hpp"
#include "nsf/NetworkMessage.hpp"
#include "nsf/Types.hpp"
#include "InternalTypes.hpp"

#include <queue>
#include <vector>
#include <map>


namespace nsf
{
struct MessageHeader;

struct ReliableMessageData
{
    NetworkMessage message;
    float sentTimeMs = 0.f;
    SequenceNumber packetSequenceNum = 0;
    SequenceNumber sequenceNum = 0;
};

class Peer final
{
public:
    Peer() = default;

    void send(const NetworkMessage& _message);
    void onMessageReceived(const MessageHeader& _header, NetworkMessage&& _message);

    PeerID getPeerId() const { return m_peerId; }

    bool hasDataToSend() const { return !m_unreliableMessagesToSend.empty() || m_hasReliableDataToSendInTheCurrentFrame; }
    
    PeerID m_peerId = PEER_ID_INVALID;

    SequenceNumber m_sequenceNumGenerator = 0;
    SequenceNumber m_sequenceNumberOfLastDelivered = MAX_SEQUENCE_NUMBER;

    bool m_hasReliableDataToSendInTheCurrentFrame = false;
    std::vector<ReliableMessageData> m_reliableMessagesToSend;     
    std::queue<NetworkMessage> m_unreliableMessagesToSend;

    std::queue<NetworkMessage> m_reliableMessagesToDeliver;
    std::queue<NetworkMessage> m_unreliableMessagesToDeliver;

    std::map<SequenceNumber, NetworkMessage> m_messagesToStore;

    static constexpr sf::Uint8 RELIABLE_CHANNEL_ID = 1;
    static constexpr sf::Uint8 UNRELIABLE_CHANNEL_ID = 2;
};

} // namespace nsf
