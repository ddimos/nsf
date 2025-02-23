#include "packetManager/PacketManager.hpp"
#include "connection/Connection.hpp"
#include "connection/InternalPacketType.hpp"
#include "connection/PacketHeader.hpp"
#include "utils/Assert.hpp"
#include "utils/Logging.hpp"
#include "utils/Utils.hpp"
#include "Constants.hpp"

#include <bitset>
#include <cmath>


namespace nsf
{
 
PacketManager::PacketManager(const sf::Clock& _systemClock, PacketManagerCallbacks _callbacks)
    : m_systemClock{_systemClock}
    , m_callbacks{_callbacks}
{
}

float PacketManager::getRtt(ConnectionID _connectionId) const
{
    for (const PacketPeer& peer : m_peers)
        if (peer.m_connectionId == _connectionId)
            return peer.m_rtt;
    
    NSF_LOG_ERROR("PacketManager::There is no peer with such id: " << _connectionId);
    NSF_ASSERT(false, "There is no peer with such id");
    return 0.f;
}

void PacketManager::onReceivePacket(ConnectionID _connectionId, PacketHeader _header)
{
    PacketPeer* peer = getPeer(_connectionId);
    if(!peer)
    {
        NSF_LOG_ERROR("PacketManager::There is no peer with such id: " << _connectionId);
        NSF_ASSERT(false, "There is no peer with such id");
        return;
    }

    peer->onReceivePacket(PacketData{0.0, _header.sequenceNum, PacketData::State::RECEIVED});
    
    float time = m_systemClock.getElapsedTime().asSeconds();
    std::unordered_set<SequenceNumber> ackedSequenceArray = peer->processAckBits(_header.ack, _header.ackBits, time);

    NSF_LOG_DEBUG("PacketManager::receive, seq: " << _header.sequenceNum << " ack: " << _header.ack << " bits: " << std::bitset<32> {_header.ackBits});

    if (!ackedSequenceArray.empty())
        m_callbacks.onPacketAcked(_connectionId, ackedSequenceArray);
}

std::tuple<SequenceNumber, SequenceNumber, AckBits> PacketManager::onSendPacket(ConnectionID _connectionId)
{
    PacketPeer* peer = getPeer(_connectionId);
    if(!peer)
    {
        NSF_LOG_ERROR("PacketManager::There is no peer with such id: " << _connectionId);
        NSF_ASSERT(false, "There is no peer with such id");
        return {};
    }

    SequenceNumber currentSequenceNum = peer->m_sequenceNumberGenerator;
    peer->m_sequenceNumberGenerator++;

    peer->onSendPacket(PacketData{m_systemClock.getElapsedTime().asSeconds(), currentSequenceNum, PacketData::State::EMPTY});
    AckBits ackBits = peer->generateAckBits();

    NSF_LOG_DEBUG("PacketManager:: send, \tseq: " << currentSequenceNum << " ack: " << peer->m_receivedSequenceNumber << " bits: " << std::bitset<32>{ackBits});

    return {currentSequenceNum, peer->m_receivedSequenceNumber ,ackBits};
}

void PacketManager::onConnected(Connection& _connection)
{
    if (getPeer(_connection.getConnectionId()))
    {
        NSF_LOG_ERROR("PacketManager::Peer already exists " << _connection.getConnectionId());
        NSF_ASSERT(false, "Peer already exists");
        return;
    }

    PacketPeer peer;
    peer.m_connectionId = _connection.getConnectionId();
    m_peers.push_back(peer);
}

void PacketManager::onDisconnected(Connection& _connection)
{
    if (!getPeer(_connection.getConnectionId()))
    {
        NSF_LOG_ERROR("PacketManager::Peer doesn't exist " << _connection.getConnectionId());
        NSF_ASSERT(false, "Peer doesn't exist ");
        return;
    }

    m_peers.erase(std::remove_if(m_peers.begin(), m_peers.end(), 
    [&_connection](const PacketPeer& _peer) {
            return _peer.m_connectionId == _connection.getConnectionId();
    }), m_peers.end());
}

PacketManager::PacketPeer* PacketManager::getPeer(ConnectionID _connectionId)
{
    for (PacketPeer& peer : m_peers)
        if (peer.m_connectionId == _connectionId)
            return &peer;
    return nullptr;
}

void PacketManager::PacketPeer::onSendPacket(PacketData _data)
{
    const int index = _data.sequence % SequencePacketBufferSize;
    m_sentPacketData[index] = _data;
}

void PacketManager::PacketPeer::onReceivePacket(PacketData _data)
{
    // for (int i = 0 ; i < d; ++i)
    // {
        // TODO is it needed to clean some entries?     
    // }
    if (sequenceGreaterThan(_data.sequence, m_receivedSequenceNumber))
        m_receivedSequenceNumber = _data.sequence;
        
    const int index = _data.sequence % SequencePacketBufferSize;
    m_receivedPacketData[index] = _data;
}

AckBits PacketManager::PacketPeer::generateAckBits()
{
    AckBits ackBits = 0;
    for (int i = 1; i <= NumberOfAckBits; ++i)
    {
        SequenceNumber seq = static_cast<SequenceNumber>(m_receivedSequenceNumber - i);
        const int index =  seq % SequencePacketBufferSize;
        ackBits <<= 1;
        if (m_receivedPacketData[index].isReceived())
        {
            ackBits |= 1;
        }

        // NSF_LOG_DEBUG("Generate ack: " << seq << "\t val: " << m_receivedPacketData[index].ackedOrReceived << "\t" << std::bitset<32>{ackBits});

        // write
        // 0000 | 0001 = 0001
        // 0001 << 1   = 0010
    }
    return ackBits;
}

std::unordered_set<SequenceNumber> PacketManager::PacketPeer::processAckBits(SequenceNumber _ack, AckBits _ackBits, float _time)
{
    std::unordered_set<SequenceNumber> ackedSequenceArray;
    ackedSequenceArray.reserve(NumberOfAckBits);

    auto processAckBit = [&](SequenceNumber _ackBit) {
        const int index = _ackBit % SequencePacketBufferSize;
        PacketData& data = m_sentPacketData[index];
        if (data.sequence == _ackBit && !data.isAcked())
        {
            data.state = PacketData::State::ACKED;
            ackedSequenceArray.emplace(_ackBit);

            // TODO read about other rtt techniques
            // this one will depend on the init value
            float rtt = _time - data.time;
            if ((m_rtt == 0.0f && rtt > 0.0f) || fabsf(m_rtt - rtt) < 0.00001f)
                m_rtt = rtt;
            else
                m_rtt += ( rtt - m_rtt ) * 0.0025f; // rtt_smoothing_factor = 0.0025f;

            NSF_LOG_DEBUG("PacketManager::RTT: " << rtt << " global: " << m_rtt);
        }
    };

    for (int i = NumberOfAckBits; i > 0; --i)
    {
        SequenceNumber ackBit = static_cast<SequenceNumber>(_ack - i);
        bool value = _ackBits & 1;
        if (value)
        {
            processAckBit(ackBit);
        }
        // NSF_LOG_DEBUG("Decode ack: " << ackSequence << "\t val: " << value << " acked: " <<  (ackedSequenceArray.find(ackBit) != ackedSequenceArray.end())  << "\t" << std::bitset<32>{_ackBits});
        
        _ackBits >>= 1;
 
        // read
        // 1010 & 0001 = 0000
        // 1010 >> 1   = 0101 
    }
   
    processAckBit(_ack);
    
    return ackedSequenceArray;
}

} // namespace nsf
