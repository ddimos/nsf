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
};

} // namespace nsf
