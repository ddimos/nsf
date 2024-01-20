#pragma once


#include "nsf/NetworkAddress.hpp"
#include "nsf/NetworkMessage.hpp"
#include "nsf/NetworkEvent.hpp"
#include "nsf/Types.hpp"

#include <memory>


namespace nsf
{


class INSF
{
public:
    virtual ~INSF() = default;

    virtual bool pollEvents(NetworkEvent& _event) = 0;
    virtual void update() = 0;

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

std::unique_ptr<INSF> createNSF(const Config& _config);

} // namespace nsf
