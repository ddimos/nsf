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

class NetworkMessage1 final
{
public:
    enum class Type
    {
        UNICAST,
        BRODCAST,
        EXCLUDE_BRODCAST
    };

    NetworkMessage1()=default;
    NetworkMessage1(bool _isReliable)
        : m_type(Type::BRODCAST), m_peerId(PEER_ID_INVALID), m_isReliable(_isReliable)
    {}
    NetworkMessage1(PeerID _peerId, bool _isReliable)
        : m_type(Type::UNICAST), m_peerId(_peerId), m_isReliable(_isReliable)
    {}
    NetworkMessage1(Type _type, PeerID _peerId, bool _isReliable)
        : m_type(_type), m_peerId(_peerId), m_isReliable(_isReliable)
    {}

    PeerID getPeerId() const { return m_peerId; }
    bool isBroadcast() const { return m_type == Type::BRODCAST; }
    bool isExcludeBroadcast() const { return m_type == Type::EXCLUDE_BRODCAST; }
    bool isUnicast() const { return m_type == Type::UNICAST; }
    bool isReliable() const { return m_isReliable; }
// TODO    bool isValid() const
    const sf::Packet& getData() const { return m_data; }
    bool isEnd() const { return m_data.endOfPacket(); }
    size_t getDataSize() const { return m_data.getDataSize(); }
    sf::Uint8 getMessageType() const { return m_messageType; }

    void setMessageType(sf::Uint8 _type) { m_messageType = _type; }
    void onReceive(sf::Packet&& _data)
    {
        m_data = _data;
    }

    template <typename T>
    void write(const T& _value) { m_data << _value; }
    template <typename T>
    void read(T& _value) {m_data >> _value; }

private:
    Type m_type = Type::BRODCAST;
    PeerID m_peerId = PEER_ID_INVALID;
    bool m_isReliable = false;
    sf::Packet m_data;
    sf::Uint8 m_messageType = 0;
};

} // namespace nsf
