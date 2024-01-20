#pragma once

#include "nsf/Types.hpp"

#include "nsf/NetworkAddress.hpp"
#include "nsf/NetworkPlayer.hpp"
#include "nsf/NetworkMessage.hpp"

namespace nsf
{

struct NetworkEvent
{
    enum class Type
    {
        ON_RECEIVE,
        ON_CONNECT_RESULT,
        ON_DISCONNECT,
    };

    NetworkEvent() = default;
    NetworkEvent(Type _type, PeerID _peerId)
    : type(_type), peerId(_peerId)
    {}

    NetworkEvent(Type _type, NetworkMessage&& _message, PeerID _peerId)
    : type(_type), message(std::move(_message)), peerId(_peerId)
    {}
    
    Type type;
    NetworkMessage message;
    PeerID peerId;
};

// struct NetworkEvent_old
// {
//     enum class Type
//     {
//         ON_RECEIVE,
//         ON_CONNECT_RESULT,
//         ON_DISCONNECT,
//         // 
//         ON_PLAYER_JOIN,
//         ON_PLAYER_LEAVE
//     };

//     NetworkEvent_old() = default;
//     NetworkEvent_old(Type _type, const NetworkPlayer& _player)
//     : type(_type), player(_player)
//     {}

//     NetworkEvent_old(Type _type, const NetworkAddress& _sender)
//     : type(_type), sender(_sender)
//     {}
//     NetworkEvent_old(Type _type, NetworkMessage&& _message, const NetworkAddress& _sender)
//     : type(_type), message(std::move(_message)), sender(_sender)
//     {}
    
//     Type type;
//     NetworkMessage message;
//     NetworkPlayer player;

//     NetworkAddress sender;
// };

} // namespace nsf
