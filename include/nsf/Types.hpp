#pragma once
#include <SFML/Config.hpp>
#include <SFML/Network/Packet.hpp>

#include <limits>

namespace nsf
{
    using Port = unsigned short;
    using PeerID = sf::Uint16;
    using PlayerID = sf::Uint16;
    using ChannelID = sf::Uint8;
    using Buffer = sf::Packet;

    inline constexpr Port PORT_INVALID = 0;
    inline constexpr PeerID PEER_ID_INVALID = std::numeric_limits<PeerID>::max();
    inline constexpr PlayerID PLAYER_ID_INVALID = std::numeric_limits<PlayerID>::max();
    inline constexpr ChannelID CHANNEL_ID_INVALID = std::numeric_limits<ChannelID>::max();

    struct Config
    {
        Port port = PORT_INVALID;
        bool isServer = false;
        int packetDropChance = 0;
        unsigned timeAfterResendReliableMessageMs = 0u;
    };

} // namespace nsf
