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

class INSF
{
public:
    virtual ~INSF() = default;

    virtual void updateReceive() = 0;
    virtual void updateSend() = 0;

    virtual void send(NetworkMessage&& _message) = 0;

    virtual void connect(NetworkAddress _address) = 0;
    virtual void disconnect() = 0;

    virtual bool isServer() const = 0;
    virtual PeerID getServerPeerId() const = 0;

    virtual NetworkAddress getPublicAddress() const = 0;
    virtual NetworkAddress getLocalAddress() const = 0;

    //virtual void createAndJoinSession(const std::string& _playerName) = 0;
    //virtual void joinSession(NetworkAddress _address, const std::string& _name) = 0;

//    virtual void leaveSession() = 0;

//    virtual bool isSessionMaster() const = 0;

};

std::unique_ptr<INSF> createNSF(const Config& _config, NSFCallbacks _callbacks);

} // namespace nsf
