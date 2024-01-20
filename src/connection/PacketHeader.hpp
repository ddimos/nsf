#pragma once
#include "connection/InternalPacketType.hpp"
#include "Constants.hpp"
#include "InternalTypes.hpp"

namespace nsf
{

struct PacketHeader
{
    PacketHeader() = default;
    PacketHeader(InternalPacketType _type);
    PacketHeader(InternalPacketType _type, SequenceNumber _seqNum, SequenceNumber _ack, AckBits _ackBits);

    void serialize(sf::Packet& _writeBuffer);
    void deserialize(sf::Packet& _readBuffer);

    // TODO protocolId
    InternalPacketType type = InternalPacketType::None;
    SequenceNumber sequenceNum = 0;
    SequenceNumber ack = 0;
    AckBits ackBits = 0;
  //  bool isReliable = false;

static constexpr size_t PacketHeaderSizeBytes = sizeof(InternalPacketType) + sizeof(SequenceNumber) + sizeof(SequenceNumber) + sizeof(AckBits);
static constexpr size_t MaxPacketPayloadBytes = MAX_PACKET_SIZE_BYTES - PacketHeaderSizeBytes;

};

static_assert(sizeof(PacketHeader) == 12, "Don't forget to change PacketHeaderSizeBytes.");

} // namespace nsf
