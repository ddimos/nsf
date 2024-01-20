#include "channel/MessageHeader.hpp"

namespace nsf
{

MessageHeader::MessageHeader(SequenceNumber _sequenceNum, sf::Uint16 _messageSize, sf::Uint8 _channelId)
    : sequenceNum(_sequenceNum), messageSize(_messageSize), channelId(_channelId)
{}

void MessageHeader::serialize(Buffer& _writeBuffer)
{
    _writeBuffer << sequenceNum;
    _writeBuffer << messageSize;
    _writeBuffer << channelId;
}

void MessageHeader::deserialize(Buffer& _readBuffer)
{
    _readBuffer >> sequenceNum;
    _readBuffer >> messageSize;
    _readBuffer >> channelId;
}

} // namespace nsf
