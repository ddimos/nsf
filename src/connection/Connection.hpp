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
    enum class Status
    {
        None,
        REQUESTING_CONNECTION,
        DECIDING_CONNECTION,
        CONNECTED,

        CONNECTION_FAILED,
        DISCONNECTING,
        DOWN

        // TODO
        // CONNECTION_REFUSED,
    };

    ConnectionID getConnectionId() const { return m_connectionId; }
    NetworkAddress getAddress() const { return m_address; }
    Status getStatus() const { return m_status; }

private:
    friend class ConnectionManager;

    ConnectionID m_connectionId = CONNECTION_ID_INVALID;

    NetworkAddress m_address = {};
    Status m_status = Status::None;
    float m_timeout = 0.f;
    int m_connectionAttemptsLeft = 0;
    float m_heartbeat = 0.f;

    bool m_connectionAccepted = false;

    std::string m_failReason = "";
};

} // namespace nsf
