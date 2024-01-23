#pragma once 
#include "nsf/Types.hpp"
#include <SFML/Network.hpp>

namespace nsf
{
struct MessageInfo final
{
    enum class Type
    {
        UNICAST,
        BRODCAST,
        EXCLUDE_BRODCAST
    };

    MessageInfo() = default;
    MessageInfo(bool _isReliable)
        : m_type(Type::BRODCAST), m_peerId(PEER_ID_INVALID), m_isReliable(_isReliable)
    {}
    MessageInfo(PeerID _peerId, bool _isReliable)
        : m_type(Type::UNICAST), m_peerId(_peerId), m_isReliable(_isReliable)
    {}
    MessageInfo(Type _type, PeerID _peerId, bool _isReliable)
        : m_type(_type), m_peerId(_peerId), m_isReliable(_isReliable)
    {}

    Type m_type = Type::BRODCAST;
    PeerID m_peerId = PEER_ID_INVALID;
    bool m_isReliable = false;
    ChannelID m_channelId = CHANNEL_ID_INVALID;
};

class NetworkMessage final
{
public:
    NetworkMessage() = default;

    PeerID getPeerId() const { return m_info.m_peerId; }
    bool isBroadcast() const { return m_info.m_type == MessageInfo::Type::BRODCAST; }
    bool isExcludeBroadcast() const { return m_info.m_type == MessageInfo::Type::EXCLUDE_BRODCAST; }
    bool isUnicast() const { return m_info.m_type == MessageInfo::Type::UNICAST; }
    bool isReliable() const { return m_info.m_isReliable; }

    MessageInfo m_info{};
    Buffer m_data;
};

} // namespace nsf
