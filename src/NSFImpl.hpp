#pragma once
#include "nsf/NetworkAddress.hpp"
#include "nsf/NetworkMessage.hpp"
#include "nsf/NSF.hpp"
#include "nsf/Types.hpp"

#include "channel/ChannelManager.hpp"
#include "connection/ConnectionManager.hpp"
#include "packetManager/PacketManager.hpp"

#include <queue>

namespace nsf
{

class NSFImpl : public INSF
{
public:
    NSFImpl(const Config& _config, NSFCallbacks _callbacks);

    void updateReceive() override;
    void updateSend() override;

    void send(NetworkMessage&& _message) override;

    void connect(NetworkAddress _address) override;
    void disconnect() override;

    bool isServer() const override;
    PeerID getServerPeerId() const override;

    NetworkAddress getPublicAddress() const override;
    NetworkAddress getLocalAddress() const override;

private:
    sf::Clock m_systemClock;
    sf::Clock m_deltaClock;

    ConnectionManager m_connectionManager;
    PacketManager m_packetManager;
    ChannelManager m_channelManager;

    Config m_config{};
    NSFCallbacks m_callbacks{};
};

} // namespace nsf
