#pragma once
#include "nsf/NetworkAddress.hpp"
#include "nsf/NetworkMessage.hpp"
#include "nsf/Types.hpp"
#include "Transport.hpp"
#include "Connection.hpp"
#include <queue>
#include <deque>
#include <map>


namespace nsf
{

class Peer : public Connection
{
public:

    Peer(Transport& _transport, NetworkAddress _addressToConnect, PeerID _peerId, bool _isCreatingFromRequest);

    void update(float _dt);
    void send(const NetworkMessage& _message);
    
    PeerID getPeerId() const { return m_peerId; }
    NetworkAddress getAddress() const { return m_address; }
    
    bool isUp() const { return getStatus() == Connection::Status::UP; }
    bool isDown() const { return getStatus() == Connection::Status::DOWN; }

    void onReliableReceived(sf::Uint32 _seqNum, const NetworkMessage& _message);
    void onAcknowledgmentReceived(sf::Uint32 _seqNum);
    std::queue<NetworkMessage>& getMessagesToDeliver() { return m_messagesToDeliver; };

private:
    static constexpr float TIME_TO_RESEND_s = 1.0f;

    struct ReliablePacketInfo
    {
        sf::Uint32 seqNum = 0;
        sf::Packet packet;
        float timeout = TIME_TO_RESEND_s;
        bool isAcknowledged = false;
    };
    void onReliableSent(sf::Packet _packet, sf::Uint32 _seqNum);
    void sendAR(sf::Uint32 _seqNum);

    PeerID m_peerId = PeerIdInvalid;

    sf::Uint32 m_sequenceNumGenerator = 0;
    sf::Uint32 m_sequenceNumberOfLastSent = 0;
    sf::Uint32 m_sequenceNumberOfLastDelivered = 0;
    std::vector<ReliablePacketInfo> m_reliableSent;
    std::queue<NetworkMessage> m_messagesToDeliver;
    std::map<sf::Uint32, NetworkMessage> m_messagesToStore;

    // TODO:
    // std::queue<NetworkMessage> m_reliableMessagesToSend;     
    // std::queue<NetworkMessage> m_unreliableMessagesToSend;     
};

} // namespace nsf
