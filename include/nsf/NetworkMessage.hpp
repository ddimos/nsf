#pragma once 
#include "nsf/Types.hpp"
#include <SFML/Network.hpp>

namespace nsf
{

class NetworkMessage final
{
public:
    enum class Type
    {
        UNICAST,
        BRODCAST,
        EXCLUDE_BRODCAST
    };

    NetworkMessage()=default;
    NetworkMessage(bool _isReliable)
        : m_type(Type::BRODCAST), m_peerId(PeerIdInvalid), m_isReliable(_isReliable)
    {}
    NetworkMessage(PeerID _peerId, bool _isReliable)
        : m_type(Type::UNICAST), m_peerId(_peerId), m_isReliable(_isReliable)
    {}
    NetworkMessage(Type _type, PeerID _peerId, bool _isReliable)
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
    PeerID m_peerId = PeerIdInvalid;
    bool m_isReliable = false;
    sf::Packet m_data;
    sf::Uint8 m_messageType = 0;
};

} // namespace nsf
