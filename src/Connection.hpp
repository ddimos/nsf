#pragma once
#include <SFML/Network.hpp>
#include "nsf/NetworkAddress.hpp"
#include "Transport.hpp"

namespace nsf
{

class Connection
{
public:
    enum class Status
    {
        None,
        CONNECTING,
        UP,
        DISCONNECTING,
        DOWN
    };

    void close(bool _forcibly = false);

    Status getStatus() const { return m_status; }
    void onConnectionAcceptReceived();
    void onHeartbeatReceived();

protected:

    Connection(Transport& _transport, NetworkAddress _addressToConnect, bool _isCreatingFromRequest);
    virtual ~Connection() = default;

    void update(float _dt);
    
    void send(sf::Packet _packet, NetworkAddress _address);

protected:
    NetworkAddress m_address = {};

private:

    static constexpr float TIME_TO_RETRY_CONNECT_s = 2.5f;
    static constexpr float HEARTBEAT_TIMEOUT_s = 10.f;
    static constexpr float HEARTBEAT_s = 1.f;
    static const int CONNECTION_ATTEMPTS = 10;

    Transport* m_transport;
    Status m_status = Status::None;
    float m_timeout = 0.f;
    int m_connectionAttemptsLeft = CONNECTION_ATTEMPTS;
    float m_heartbeat = HEARTBEAT_s;
};

} // namespace nsf
