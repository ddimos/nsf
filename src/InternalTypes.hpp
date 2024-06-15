#pragma once
#include "nsf/Types.hpp"

#include <type_traits>
#include <limits>
#include <cstdint>

namespace nsf
{
    using ConnectionID = PeerID;
    using SequenceNumber = sf::Uint16;
    static_assert(std::is_unsigned<SequenceNumber>::value, "SequenceNumber must be an unsigned type.");

    using AckBits = sf::Uint32;
    
    inline constexpr ConnectionID CONNECTION_ID_INVALID = PEER_ID_INVALID;
    inline constexpr SequenceNumber MAX_SEQUENCE_NUMBER = std::numeric_limits<SequenceNumber>::max();

} // namespace nsf

