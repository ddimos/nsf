#pragma once


#include "nsf/NetworkAddress.hpp"
#include "nsf/NetworkMessage.hpp"
#include "nsf/NetworkEvent.hpp"
#include "nsf/Types.hpp"

#include <memory>


namespace nsf
{

struct Config
{
    Port port = 0;
};

class INSF
{
public:
    virtual ~INSF() = default;

    virtual bool pollEvents(NetworkEvent& _event) = 0;
    virtual void update(float _dt) = 0;

    virtual void send(const NetworkMessage& _message) = 0;

    virtual void createAndJoinSession(const std::string& _playerName) = 0;
    virtual void joinSession(NetworkAddress _address, const std::string& _name) = 0;

    virtual void leaveSession() = 0;

    virtual bool isSessionMaster() const = 0;
    virtual PeerID getHostPeerId() const = 0;

    virtual NetworkAddress getPublicAddress() const = 0;
    virtual NetworkAddress getLocalAddress() const = 0;
};

std::unique_ptr<INSF> createNSF(const Config& _config);

} // namespace nsf
