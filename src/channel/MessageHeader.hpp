#pragma once
#include "connection/PacketHeader.hpp"

namespace nsf
{

struct MessageHeader
{
    MessageHeader() = default;
    MessageHeader(SequenceNumber _sequenceNum, sf::Uint16 _messageSize, sf::Uint8 _channelId);

    void serialize(Buffer& _writeBuffer);
    void deserialize(Buffer& _readBuffer);

    SequenceNumber sequenceNum = 0; // TODO optimize it for unreliable
    sf::Uint16 messageSize = 0;
    sf::Uint8 channelId = 0;

    static constexpr size_t MessageHeaderSizeBytes = sizeof(SequenceNumber) + sizeof(sf::Uint16) + sizeof(sf::Uint8);
    static constexpr size_t MaxMessageSizeBytes = PacketHeader::MaxPacketPayloadBytes - MessageHeaderSizeBytes;
};

static_assert(sizeof(MessageHeader) == 6, "Don't forget to change MessageHeaderSizeBytes.");

} // namespace nsf
