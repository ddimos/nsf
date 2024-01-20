#pragma once

#include <SFML/Network.hpp>

namespace nsf
{

enum class InternalPacketType : sf::Uint8
{
    None,
    CONNECT_REQUEST,
    CONNECT_ACCEPT,
    DISCONNECT,
    HEARTBEAT,
    USER_PACKET
    
    // TODO
    // SESSION_JOIN_REQUEST,
    // SESSION_JOIN_ACCEPT,
    // SESSION_ON_JOIN,
    // SESSION_ON_LEAVE,
};

} // namespace nsf
