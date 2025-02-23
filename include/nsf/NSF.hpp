#pragma once

#include "nsf/NetworkAddress.hpp"
#include "nsf/NetworkMessage.hpp"
#include "nsf/Types.hpp"

#include <functional>
#include <memory>

namespace nsf
{

struct NSFCallbacks
{
    std::function<void(PeerID _peerId)> onConnected{};
    std::function<void(PeerID _peerId)> onDisconnected{};
    std::function<void(NetworkMessage&& _message)> onReceived{};
};

// TODO Write more documentation. 
class INSF
{
public:
    virtual ~INSF() = default;

    //////////////////////////////////////////
    /// Receive data from the socket and process it.
    /// 
    /// NSFCallbacks will be triggered during this update.
    ///
    //////////////////////////////////////////
    virtual void updateReceive() = 0;

    //////////////////////////////////////////
    /// Send data to the socket.
    /// 
    //////////////////////////////////////////
    virtual void updateSend() = 0;

    //////////////////////////////////////////
    /// Add the message to a queue to send.
    ///
    /// It will be processed during updateSend.
    /// 
    /// \param _message     The message to send 
    ///
    //////////////////////////////////////////
    virtual void send(NetworkMessage&& _message) = 0;

    //////////////////////////////////////////
    /// Try to connect to a remote peer.
    /// 
    /// \param _address     Address of the remote peer.
    /// 
    //////////////////////////////////////////
    virtual void connect(NetworkAddress _address) = 0;

    //////////////////////////////////////////
    /// Gracefully disconnect from a remote peer.
    ///
    /// \param _peerId      An id of the peer to disconnect.
    /// If the default argument is used, the program will disconnect from everyone.
    ///
    //////////////////////////////////////////
    virtual void disconnect(PeerID _peerId = PEER_ID_INVALID) = 0;

    //////////////////////////////////////////
    /// Tell whether the instance is the server.
    ///
    //////////////////////////////////////////
    virtual bool isServer() const = 0;

    //////////////////////////////////////////
    /// Get the server peer id.
    /// 
    /// On the server it will return PEER_ID_INVALID.
    ///
    //////////////////////////////////////////
    virtual PeerID getServerPeerId() const = 0;

    //////////////////////////////////////////
    /// Get a round-trip time of a remote peer in seconds.
    ///
    /// \param _peerId      An id of the peer to get the rtt.
    ///
    //////////////////////////////////////////
    virtual float getRtt(PeerID _peerId) const = 0;

    //////////////////////////////////////////
    /// Get the local address and the port the socket is bound to. 
    /// 
    //////////////////////////////////////////
    virtual NetworkAddress getPublicAddress() const = 0;

    //////////////////////////////////////////
    /// Get the public address and the port the socket is bound to. 
    ///
    //////////////////////////////////////////
    virtual NetworkAddress getLocalAddress() const = 0;
};

std::unique_ptr<INSF> createNSF(const Config& _config, NSFCallbacks _callbacks);

} // namespace nsf
