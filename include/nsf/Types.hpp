#pragma once
#include <SFML/Config.hpp>

namespace nsf
{

    using Port = unsigned short;
    constexpr Port PortInvalid = 0;

    using PeerID = sf::Uint16;
    using PlayerID = sf::Uint16;
    const PeerID PeerIdInvalid = 65535; 
    const PlayerID PlayerIdInvalid = 65535; 

} // namespace nsf
