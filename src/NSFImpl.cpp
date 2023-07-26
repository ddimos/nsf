// #include "Utils/Log.h"
#include "NSFImpl.hpp"

#include "InternalPacketType.hpp"
#include "PacketHeader.hpp"

namespace nsf
{

std::unique_ptr<INSF> createNSF(const Config& _config)
{
    return std::make_unique<NSFImpl>(_config);
}

NSFImpl::NSFImpl(const Config& _config)
    : Transport(_config.port)
{
    m_peers.reserve(10);
    m_players.reserve(10); // TODO use ptrs
}

bool NSFImpl::pollEvents(NetworkEvent& _event)
{
    if (m_events.empty())
        return false; 
    
    _event = m_events.front();
    m_events.pop();

    return true;
}

void NSFImpl::update(float _dt)
{
    Transport::update();
    
    for (Peer& peer : m_peers)
    {
        peer.update(_dt);
    }

    m_peers.erase(std::remove_if(m_peers.begin(), m_peers.end(), 
    [this](const Peer& _peer)
    {
        if (_peer.isDown())
        {
            onDisconnect(_peer);
            return true;
        }
        return false;
    }), m_peers.end());

    m_players.erase(std::remove_if(m_players.begin(), m_players.end(),
        [](const NetworkPlayer& _player) { return _player.isLeft(); }), m_players.end());
}

void NSFImpl::send(const NetworkMessage& _message)
{
    if (_message.isBroadcast())
    {
        for (Peer& peer : m_peers)
        {
            if (peer.isUp())
                peer.send(_message);
            //else
                //LOG("Don't send the message to the peer " + tstr(_message.GetPeerId()) + " because it isn't up.");
        }
    }
    else if(_message.isExcludeBroadcast())
    {
        for (Peer& peer : m_peers)
        {
            if (peer.getPeerId() == _message.getPeerId())
                continue;
            
            if (peer.isUp())
                peer.send(_message);
            //else
                //LOG("Don't send the message to the peer " + tstr(_message.GetPeerId()) + " because it isn't up.");
        }
    }
    else
    {
        Peer* peer = getPeer(_message.getPeerId());
        if (peer && peer->isUp())
            peer->send(_message);
        //else
            //LOG("Don't send the message to the peer " + tstr(_message.GetPeerId()) + " because it isn't up.");
    }
}

void NSFImpl::createAndJoinSession(const std::string& _playerName)
{
    m_isSessionMaster = true;
    m_isSessionCreated = true;  // TODO: onSessionCreatedEvent

    m_localPlayer = createPlayerIntrernal(_playerName, ++m_playerIdGenerator, true);
    m_events.emplace(NetworkEvent(NetworkEvent::Type::ON_PLAYER_JOIN, *m_localPlayer)); // it's a copy
}

void NSFImpl::joinSession(NetworkAddress _address, const std::string& _playerName)
{
    if (m_isSessionMaster || m_connectingToHost)
        return;
    
    m_connectingToHost = true;
    m_localPlayer = createPlayerIntrernal(_playerName, PlayerIdInvalid, true);
    connect(_address);
}

void NSFImpl::leaveSession()
{
    disconnect();
    m_isSessionCreated = false;
    m_connectingToHost = false;
    m_isSessionMaster = false;
    m_localPlayer = nullptr;
}

NetworkAddress NSFImpl::getPublicAddress() const
{
    return Transport::getPublicAddress();
}

NetworkAddress NSFImpl::getLocalAddress() const
{
    return Transport::getLocalAddress();
}

void NSFImpl::onReceivePacket(sf::Packet _packet, NetworkAddress _sender)
{
    Peer* senderPeer = getPeer(_sender);
 
    PacketHeader header;
    header.deserialize(_packet);

    switch (header.type)
    {
    case InternalPacketType::INTERNAL_CONNECT_REQUEST:
    {
        if (senderPeer)
        {
            //LOG_ERROR("CONNECT_REQUEST received again from " + _sender.toString());
            break;
        }
        if (!m_isSessionMaster)
        {
            //LOG_ERROR("Cannot process CONNECT_REQUEST recieved from " + _sender.toString() + " because not the session master");
            break;
        }
        
        //LOG("CONNECT_REQUEST received from " + _sender.toString());

        Peer& peer = createPeerInternal(_sender, true);
        onConnect(peer);
        break;
    }
    case InternalPacketType::INTERNAL_CONNECT_ACCEPT:
    {
        if (!senderPeer)
        {
            //LOG_ERROR("CONNECT_ACCEPT received from " + _sender.toString() + " who we didn't ask");
            break;
        }
        else if (senderPeer->getStatus() != Peer::Status::CONNECTING)
        {
            //LOG_ERROR("The status of " + _sender.toString() + " isn't CONNECTING");
            break;
        }
        //LOG("CONNECT_ACCEPT received from " + _sender.toString());
        senderPeer->onConnectionAcceptReceived();
        onConnect(*senderPeer);
        break;
    }
    case InternalPacketType::INTERNAL_DISCONNECT:
    {
        if (!senderPeer)
        {
            //LOG_ERROR("DISCONNECT received from " + _sender.toString() + " who is not in the list of peers");
            break;
        }
        else if (senderPeer->getStatus() == Peer::Status::DOWN)
        {
            //LOG_ERROR("The status of " + _sender.toString() + " is already DOWN");
            break;
        }
        //LOG("DISCONNECT received from " + _sender.toString());
        senderPeer->close(true);
        break;
    }
    case InternalPacketType::INTERNAL_HEARTBEAT:
    {
        if (!senderPeer)
        {
            //LOG_ERROR("Received from " + _sender.toString() + " who is not in the list of peers");
            break;
        }
        else if (senderPeer->getStatus() == Peer::Status::CONNECTING)
        {
            senderPeer->onConnectionAcceptReceived();
            onConnect(*senderPeer);
            //LOG("Received a heartbeat from " + _sender.toString() + " while waiting for connection accept");
        }
        else if (!senderPeer->isUp())
        {
            //LOG_ERROR("Received from " + _sender.toString() + " who is not UP");
            break;
        }
        senderPeer->onHeartbeatReceived();
        //LOG_DEBUG("Received a heartbeat from " + _sender.toString());
        break;
    }
    case InternalPacketType::INTERNAL_AR:
    {
        if (!senderPeer)
        {
            //LOG_ERROR("Received from " + _sender.toString() + " who is not in the list of peers");
            break;
        }
        else if (!senderPeer->isUp())
        {
            //LOG_ERROR("Received from " + _sender.toString() + " who is not UP");
            break;
        }
        //LOG_DEBUG("AR received from " + _sender.toString() + " seqNum: " + tstr(header.sequenceNum));

        senderPeer->onAcknowledgmentReceived(header.sequenceNum);

        break;
    }

    case InternalPacketType::INTERNAL_SESSION_JOIN_REQUEST:
    case InternalPacketType::INTERNAL_SESSION_JOIN_ACCEPT:
    case InternalPacketType::INTERNAL_SESSION_ON_JOIN:
    case InternalPacketType::INTERNAL_SESSION_ON_LEAVE:
    case InternalPacketType::USER_PACKET:
    {
        if (!senderPeer)
        {
            //LOG_ERROR("Received from " + _sender.toString() + " who is not in the list of peers");
            break;
        }
        else if (!senderPeer->isUp())
        {
            //LOG_ERROR("Received from " + _sender.toString() + " who is not UP");
            break;
        }
        //LOG_DEBUG("Received from " + _sender.toString());

        NetworkMessage message(senderPeer->getPeerId(), header.isReliable);
        message.setMessageType(static_cast<sf::Uint8>(header.type));
        message.onReceive(std::move(_packet));

        if (header.isReliable)
        {
            senderPeer->onReliableReceived(header.sequenceNum, message);
            auto& messages = senderPeer->getMessagesToDeliver();
            while(!messages.empty())
            {
                auto& mes = messages.front();   // It's a real mess to have different types here
                InternalPacketType messageType = static_cast<InternalPacketType>(mes.getMessageType());
                if (messageType == InternalPacketType::INTERNAL_SESSION_JOIN_REQUEST)
                    processSessionJoinRequest(mes, senderPeer);
                else if (messageType == InternalPacketType::INTERNAL_SESSION_JOIN_ACCEPT)
                    processSessionJoinAccept(mes);
                else if (messageType == InternalPacketType::INTERNAL_SESSION_ON_JOIN)
                    processSessionOnJoin(mes);
                else if (messageType == InternalPacketType::INTERNAL_SESSION_ON_LEAVE)
                    processSessionOnLeave(mes);
                else
                    m_events.push(NetworkEvent(NetworkEvent::Type::ON_RECEIVE, std::move(mes), _sender));
                
                messages.pop();
            }
        }
        else
        {
            m_events.push(NetworkEvent(NetworkEvent::Type::ON_RECEIVE, std::move(message), _sender));
        }
        
        break;
    }
    default:
        break;
    }
}

void NSFImpl::connect(NetworkAddress _addressToConnect)
{
    if (getPeer(_addressToConnect) != nullptr)
    {
        //LOG_ERROR("Don't connect to " + _addressToConnect.toString() + " because it is already connected.");
        return;
    }
    //LOG("Connect to " + _addressToConnect.toString());
    createPeerInternal(_addressToConnect, false);
}

void NSFImpl::disconnect()
{
    for (Peer& peer : m_peers)
        peer.close();
    
    //LOG("Disconnect");

    if (m_isSessionMaster && m_peers.empty())
    {
        m_localPlayer->onLeft();
        m_events.emplace(NetworkEvent(NetworkEvent::Type::ON_PLAYER_LEAVE, *m_localPlayer));
    }
}

Peer* NSFImpl::getPeer(NetworkAddress _address)
{
    for (Peer& peer : m_peers)
        if (peer.getAddress() == _address)
            return &peer;
        
    return nullptr;
}

Peer* NSFImpl::getPeer(PeerID _peerId)
{
    for (Peer& peer : m_peers)
        if (peer.getPeerId() == _peerId)
            return &peer;
        
    return nullptr;
}

void NSFImpl::onConnect(Peer& _peer)
{
    if (m_isSessionMaster)
        return;
    
    m_hostPeerId = _peer.getPeerId();

    NetworkMessage message(m_hostPeerId, true);
    message.setMessageType(static_cast<sf::Uint8>(InternalPacketType::INTERNAL_SESSION_JOIN_REQUEST));
    message.write(m_localPlayer->getName());
    //LOG_DEBUG("Send a join session request to " + tstr(_peer.GetPeerId()));  
    send(message);
}

void NSFImpl::onDisconnect(const Peer& _peer)
{
    if (m_isSessionMaster)
    {
        for (NetworkPlayer& player : m_players)
        {
            if (!player.isLocal() && player.getPeerId() == _peer.getPeerId())
            {
                NetworkMessage message(true);
                message.setMessageType(static_cast<sf::Uint8>(InternalPacketType::INTERNAL_SESSION_ON_LEAVE));
                message.write(player.getPlayerId());
                //LOG_DEBUG("Send a session leave message");  
                send(message);
                m_events.emplace(NetworkEvent(NetworkEvent::Type::ON_PLAYER_LEAVE, player));
                player.onLeft();
                break;
            }
        }
    }
    else
    {
        // All players leave
        for (NetworkPlayer& player : m_players) 
        {
            player.onLeft();
            m_events.emplace(NetworkEvent(NetworkEvent::Type::ON_PLAYER_LEAVE, player));
        }
    }
}

Peer& NSFImpl::createPeerInternal(NetworkAddress _addressToConnect, bool _isCreatingFromRequest)
{
    PeerID peerId = ++m_peerIdGenerator;
    //LOG("Create a new peer. id: " + tstr(peerId) + " address: " + _addressToConnect.toString());
    return m_peers.emplace_back(Peer(*this, _addressToConnect, peerId, _isCreatingFromRequest));
}

NetworkPlayer* NSFImpl::createPlayerIntrernal(const std::string& _name, PlayerID _id, bool _isLocal)
{
    auto& player = m_players.emplace_back(NetworkPlayer(_name, _id, _isLocal));
    return &player;
}

void NSFImpl::processSessionJoinRequest(NetworkMessage& _message, Peer* _peer)
{
    if (!m_isSessionMaster)
    {
        //LOG_ERROR("Cannot process the JoinRequest recieved from " + tstr(_message.GetPeerId()) + " because not the session master");
        return;
    }
    //LOG_DEBUG("JoinRequest received from " + tstr(_message.GetPeerId()));

    std::string newPlayerName;
    _message.read(newPlayerName);
    PlayerID newPlayerId = ++m_playerIdGenerator;

    NetworkMessage message(NetworkMessage::Type::UNICAST, _peer->getPeerId(), true);
    message.setMessageType(static_cast<sf::Uint8>(InternalPacketType::INTERNAL_SESSION_JOIN_ACCEPT));
    message.write(newPlayerName);
    message.write(newPlayerId);
    for (const auto& player : m_players)
    {
        message.write(player.getName());
        message.write(player.getPlayerId());
    }
    
    //LOG_DEBUG("Send a join session accept to " + tstr(_peer->GetPeerId()));  
    send(message);   

    NetworkMessage messageOnJoin(NetworkMessage::Type::EXCLUDE_BRODCAST, _peer->getPeerId(), true);
    messageOnJoin.setMessageType(static_cast<sf::Uint8>(InternalPacketType::INTERNAL_SESSION_ON_JOIN));
    messageOnJoin.write(newPlayerName);
    messageOnJoin.write(newPlayerId);
     
    //LOG_DEBUG("Send a session join message");  
    send(messageOnJoin);   

    auto* player = createPlayerIntrernal(newPlayerName, newPlayerId, false);
    player->setPeerId(_peer->getPeerId()); // I guess I need this only on the host side
    m_events.emplace(NetworkEvent(NetworkEvent::Type::ON_PLAYER_JOIN, *player));
}

void NSFImpl::processSessionJoinAccept(NetworkMessage& _message)
{
    if (m_isSessionMaster)
    {
        //LOG_ERROR("The session master should not receive a JoinAccept");
        return;
    }

    //LOG_DEBUG("JoinAccept received from " + tstr(_message.GetPeerId()));

    std::string playerName;
    PlayerID playerId;
    _message.read(playerName);
    _message.read(playerId);

    if (m_localPlayer->getName() != playerName)
    {
        //LOG_ERROR("The local player name is not equal to the one received from the host: " + m_localPlayer->m_name + " != " + playerName);
        return;
    }
    m_localPlayer->setPlayerId(playerId);
    m_events.emplace(NetworkEvent(NetworkEvent::Type::ON_PLAYER_JOIN, *m_localPlayer));
    while (!_message.isEnd())
    {
        _message.read(playerName);
        _message.read(playerId);
        auto player = createPlayerIntrernal(playerName, playerId, false);
        m_events.emplace(NetworkEvent(NetworkEvent::Type::ON_PLAYER_JOIN, *player));
    }
}

void NSFImpl::processSessionOnJoin(NetworkMessage& _message)
{
    if (m_isSessionMaster)
    {
        //LOG_ERROR("The session master should not receive a JoinAccept");
        return;
    }

    //LOG_DEBUG("OnJoin received");

    std::string playerName;
    PlayerID playerId;
    _message.read(playerName);
    _message.read(playerId);

    auto player = createPlayerIntrernal(playerName, playerId, false);
    m_events.emplace(NetworkEvent(NetworkEvent::Type::ON_PLAYER_JOIN, *player));
}

void NSFImpl::processSessionOnLeave(NetworkMessage& _message)
{
    if (m_isSessionMaster)
    {
        //LOG_ERROR("The session master should not receive a JoinAccept");
        return;
    }

    //LOG_DEBUG("OnLeave received");

    PlayerID playerId;
    _message.read(playerId);

    for (NetworkPlayer& player : m_players)
    {
        if (player.getPlayerId() == playerId)
        {
            m_events.emplace(NetworkEvent(NetworkEvent::Type::ON_PLAYER_LEAVE, player));
            player.onLeft();
            break;
        }
    }
}
    
} // namespace nsf
