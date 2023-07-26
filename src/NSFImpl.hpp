#pragma once
#include "nsf/NSF.hpp"
#include "Transport.hpp"
#include "nsf/NetworkAddress.hpp"
#include "nsf/NetworkMessage.hpp"
#include "nsf/NetworkEvent.hpp"
#include "nsf/NetworkPlayer.hpp"
#include "nsf/Types.hpp"
#include "Peer.hpp"
#include "Transport.hpp"

#include <queue>
#include <vector>


namespace nsf
{


class NSFImpl : public INSF, public Transport
{
public:
    NSFImpl(const Config& _config);

    bool pollEvents(NetworkEvent& _event) override;
    void update(float _dt) override;

    void send(const NetworkMessage& _message) override;

    void createAndJoinSession(const std::string& _playerName) override;  // Should be just CreateSession when I have a server
    void joinSession(NetworkAddress _address, const std::string& _name) override;

    void leaveSession() override;

    bool isSessionMaster() const override { return m_isSessionMaster; }
    PeerID getHostPeerId() const override { return m_hostPeerId; }

    NetworkAddress getPublicAddress() const override;
    NetworkAddress getLocalAddress() const override;

    const std::vector<Peer>& getPeers() const { return m_peers; }

private:

    void onReceivePacket(sf::Packet _packet, NetworkAddress _sender) override;
    
    Peer* getPeer(NetworkAddress _address);
    Peer* getPeer(PeerID _peerId);

    void connect(NetworkAddress _addressToConnect);
    void disconnect(); 

    void onConnect(Peer& _peer);
    void onDisconnect(const Peer& _peer);

    void processSessionJoinRequest(NetworkMessage& _message, Peer* _peer);
    void processSessionJoinAccept(NetworkMessage& _message);
    void processSessionOnJoin(NetworkMessage& _message);
    void processSessionOnLeave(NetworkMessage& _message);
    
    Peer& createPeerInternal(NetworkAddress _addressToConnect, bool _isCreatingFromRequest);

    std::queue<NetworkEvent> m_events;
    std::vector<Peer>   m_peers;
    PeerID m_hostPeerId = PeerIdInvalid;

    // session
    NetworkPlayer* createPlayerIntrernal(const std::string& _name, PlayerID _id, bool _isLocal);
    std::vector<NetworkPlayer> m_players; // TODO sharedPtr
    NetworkPlayer* m_localPlayer = nullptr;

    bool m_isSessionMaster = false;
    bool m_isSessionCreated = false;
    bool m_connectingToHost = false;

    PeerID m_peerIdGenerator = 0;
    PlayerID m_playerIdGenerator = 0;

    std::string m_pendingPlayerToJoin;
};

} // namespace nsf
