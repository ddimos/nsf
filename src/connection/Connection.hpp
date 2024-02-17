#pragma once
#include <SFML/Network.hpp>
#include "nsf/NetworkAddress.hpp"
#include "InternalTypes.hpp"

namespace nsf
{
class ConnectionManager;

class Connection
{
public:
    enum class State
    {
        None,
        REQUESTING_CONNECTION,
        DECIDING_CONNECTION,
        CONNECTED,

        CONNECTION_FAILED,
        DISCONNECTING,
        DISCONNECTED

        // TODO
        // CONNECTION_REFUSED,
    };

    ConnectionID getConnectionId() const { return m_connectionId; }
    NetworkAddress getAddress() const { return m_address; }
    State getState() const { return m_state; }

    bool isConnected() const { return m_state == State::CONNECTED; }
    bool isDisconnected() const { return m_state == State::DISCONNECTED; }

private:
    friend class ConnectionManager;
    
    void setState(State _state) { m_state = _state; }

    ConnectionID m_connectionId = CONNECTION_ID_INVALID;

    NetworkAddress m_address = {};
    State m_state = State::None;
    unsigned m_connectionRequestSentTimeMs = 0u;
    int m_connectionAttemptsLeft = 0;
    uint32_t m_heartbeatSentTimeMs = 0;
    uint32_t m_heartbeatReceivedTimeMs = 0;

    bool m_connectionAccepted = false;
    bool m_wasConnected = false;

    std::string m_failReason = "";
};

} // namespace nsf
