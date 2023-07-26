// #include "Utils/Log.h"
#include "Peer.hpp"
#include "PacketHeader.hpp"
#include <algorithm>

namespace nsf
{

Peer::Peer(Transport& _transport, NetworkAddress _addressToConnect, PeerID _peerId, bool _isCreatingFromRequest)
    : Connection(_transport, _addressToConnect, _isCreatingFromRequest)
    , m_peerId(_peerId)
{
}

void Peer::update(float _dt)
{
    Connection::update(_dt);

    m_reliableSent.erase(
        std::remove_if(m_reliableSent.begin(), m_reliableSent.end(),
            [](const auto& _info)
            {
                return _info.isAcknowledged;
            })
        , m_reliableSent.end());   
    

    for (ReliablePacketInfo& _info : m_reliableSent)
    {
        _info.timeout -= _dt;
        if (_info.timeout <= 0.f)
        {
            Connection::send(_info.packet, m_address);
            //LOG_DEBUG("onReliableSent again. Sequence number: " + tstr(_info.seqNum));
            _info.timeout = TIME_TO_RESEND_s;
        }   
    }
}

void Peer::send(const NetworkMessage& _message)
{
    sf::Packet packet;
    sf::Uint32 seqNum = _message.isReliable() ? ++m_sequenceNumGenerator : 0;
    PacketHeader header(static_cast<InternalPacketType>(_message.getMessageType()), _message.isReliable(), seqNum);
    header.serialize(packet);
    packet.append(_message.getData().getData(),_message.getData().getDataSize());

    Connection::send(packet, m_address);

    if (_message.isReliable())
        onReliableSent(packet, seqNum);
}

void Peer::onReliableSent(sf::Packet _packet, sf::Uint32 _seqNum)
{
    //LOG_DEBUG("onReliableSent. Sequence number: " + tstr(_seqNum));
    auto it = std::find_if(m_reliableSent.begin(), m_reliableSent.end(), 
                            [_seqNum](const ReliablePacketInfo& _info) { return _info.seqNum == _seqNum; });
    
    if (it == m_reliableSent.end())
        m_reliableSent.emplace_back(ReliablePacketInfo{_seqNum, _packet, TIME_TO_RESEND_s, false});
}

void Peer::sendAR(sf::Uint32 _seqNum)
{
    //LOG_DEBUG("Send AR. Sequence number: " + tstr(_seqNum));
    sf::Packet packet;
    PacketHeader header(InternalPacketType::INTERNAL_AR, false, _seqNum);
    header.serialize(packet);
    Connection::send(packet, m_address);
}

void Peer::onReliableReceived(sf::Uint32 _seqNum, const NetworkMessage& _message)
{
    sendAR(_seqNum);
        // TODO:
        // send also m_sequenceNumberOfLastReceived
        // maybe also m_sequenceNumberOfLastSent

    if (_seqNum <= m_sequenceNumberOfLastDelivered) // Drop
    {
        //LOG("Drop the message. Received sequence number: " + tstr(_seqNum) + " sequence number of last delivered: " + tstr(m_sequenceNumberOfLastDelivered));
        return;
    }
    
    if (_seqNum - m_sequenceNumberOfLastDelivered > 1) // Store
    {
        //LOG("Store the message. Received sequence number: " + tstr(_seqNum) + " sequence number of last delivered: " + tstr(m_sequenceNumberOfLastDelivered));
        m_messagesToStore.insert({_seqNum, _message});
        return;
    }    

    m_messagesToDeliver.push(_message);
    ++m_sequenceNumberOfLastDelivered;

    //LOG_DEBUG("Deliver message seqNum: " + tstr(_seqNum)+ " sequence number of last delivered: " + tstr(m_sequenceNumberOfLastDelivered));
    for (const auto& [storeSeqNum, storeMessage] : m_messagesToStore)
    {
        //LOG_DEBUG("Stored message seqNum: " + tstr(storeSeqNum));
        if (storeSeqNum - m_sequenceNumberOfLastDelivered == 1)
        {
            m_messagesToDeliver.push(storeMessage);
            ++m_sequenceNumberOfLastDelivered;
            //LOG_DEBUG("Deliver message from stored seqNum: " + tstr(storeSeqNum)+ " sequence number of last delivered: " + tstr(m_sequenceNumberOfLastDelivered));
        }
    }

    for (auto it = m_messagesToStore.begin(); it != m_messagesToStore.end();)
    {
        if (it->first <= m_sequenceNumberOfLastDelivered)
            it = m_messagesToStore.erase(it);
        else
            ++it;
    }
}

void Peer::onAcknowledgmentReceived(sf::Uint32 _seqNum)
{
    auto it = std::find_if(m_reliableSent.begin(), m_reliableSent.end(),
        [_seqNum](const auto& _info)
        {
            return _seqNum == _info.seqNum;
        });   
    if (it != m_reliableSent.end())
        it->isAcknowledged = true;
}

} // namespace nsf
