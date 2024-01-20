#include "connection/PacketHeader.hpp"

namespace nsf
{

PacketHeader::PacketHeader(InternalPacketType _type)
    : type{_type}
{
}

PacketHeader::PacketHeader(InternalPacketType _type, SequenceNumber _seqNum, SequenceNumber _ack, AckBits _ackBits)
    : type{_type}, sequenceNum{_seqNum}, ack{_ack}, ackBits{_ackBits}
{
}

void PacketHeader::serialize(sf::Packet& _writeBuffer)
{
    _writeBuffer << static_cast<sf::Uint8>(type);
    _writeBuffer << sequenceNum;
    _writeBuffer << ack;
    _writeBuffer << ackBits;
}

void PacketHeader::deserialize(sf::Packet& _readBuffer)
{
    sf::Uint8 t;
    _readBuffer >> t;
    type = static_cast<InternalPacketType>(t);
    _readBuffer >> sequenceNum;
    _readBuffer >> ack;
    _readBuffer >> ackBits;
}

} // namespace nsf
