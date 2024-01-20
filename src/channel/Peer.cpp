#include "channel/Peer.hpp"
#include "channel/MessageHeader.hpp"
#include "utils/Assert.hpp"
#include "utils/Logging.hpp"
#include "utils/Utils.hpp"
#include <algorithm>

namespace nsf
{

void Peer::send(const NetworkMessage& _message)
{
    if (_message.isReliable())
    {
        if (_message.m_data.getDataSize() > MessageHeader::MaxMessageSizeBytes)
        {
            NSF_LOG_ERROR("Peer::The message is too large to fit one packet: " << _message.m_data.getDataSize());
            NSF_ASSERT(false, "The message is too large to fit one packet");
            return;
        }
        m_hasReliableDataToSendInTheCurrentFrame = true;
        m_reliableMessagesToSend.push_back(ReliableMessageData{ _message, 0.f, 0, m_sequenceNumGenerator++ });
    }
    else
    {
        if (_message.m_data.getDataSize() > MessageHeader::MaxMessageSizeBytes)
        {
            NSF_LOG_ERROR("Peer::The message is too large to fit one packet: " << _message.m_data.getDataSize());
            NSF_ASSERT(false, "The message is too large to fit one packet");
            return;
        }
        m_unreliableMessagesToSend.emplace(_message);
    }
}

void Peer::onMessageReceived(const MessageHeader& _header, NetworkMessage&& _message)
{
    if (_header.channelId == UNRELIABLE_CHANNEL_ID)
        m_unreliableMessagesToDeliver.emplace(std::move(_message));
    else if (_header.channelId == RELIABLE_CHANNEL_ID)
    {
        if (sequenceEqualOrGreaterThan(m_sequenceNumberOfLastDelivered, _header.sequenceNum))
        {
            NSF_LOG_DEBUG("Peer::Drop the message: " << _header.sequenceNum);
            return;
        }

        if (_header.sequenceNum - m_sequenceNumberOfLastDelivered > 1)
        {
            NSF_LOG_DEBUG("Peer::Store the message. Received sequence number: " << _header.sequenceNum << " sequence number of last delivered: " << m_sequenceNumberOfLastDelivered);
            m_messagesToStore.emplace(_header.sequenceNum, std::move(_message));
            return;
        }

        m_reliableMessagesToDeliver.emplace(std::move(_message));
        m_sequenceNumberOfLastDelivered = _header.sequenceNum;

        NSF_LOG_DEBUG("Peer::Deliver message seqNum: " << _header.sequenceNum << " sequence number of last delivered: " << m_sequenceNumberOfLastDelivered);
        for (auto& [storeSeqNum, storeMessage] : m_messagesToStore)
        {
            NSF_LOG_DEBUG("Peer::Stored message seqNum: " + tstr(storeSeqNum));
            if (storeSeqNum - m_sequenceNumberOfLastDelivered == 1)
            {
                m_reliableMessagesToDeliver.emplace(std::move(storeMessage));
                ++m_sequenceNumberOfLastDelivered;
                NSF_LOG_DEBUG("Peer::Deliver message from stored seqNum: " << storeSeqNum << " sequence number of last delivered: " << m_sequenceNumberOfLastDelivered);
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
}

} // namespace nsf
