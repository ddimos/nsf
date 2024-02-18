#pragma once

#include "nsf/Types.hpp"

#include <SFML/Network.hpp>
#include <string>

namespace nsf
{

struct NetworkAddress
{
    NetworkAddress() = default;
    NetworkAddress(sf::IpAddress _ip) 
        : address(_ip) {}
    NetworkAddress(sf::IpAddress _ip, Port _port) 
        : address(_ip), port(_port) {}

    sf::IpAddress   address = {};
    Port            port = PORT_INVALID;
    std::string toString() const { return address.toString()+":"+std::to_string(port); }
};
inline bool operator==(const NetworkAddress& _lhs, const NetworkAddress& _rhs)
{
    return _lhs.address == _rhs.address && _lhs.port == _rhs.port;
}
inline bool operator!=(const NetworkAddress& _lhs, const NetworkAddress& _rhs)
{
    return !(_lhs == _rhs);
}

} // namespace nsf
