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
        DISCONNECTED

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
    unsigned m_connectionRequestSentTimeMs = 0u;
    int m_connectionAttemptsLeft = 0;
    uint32_t m_heartbeatSentTimeMs = 0;
    uint32_t m_heartbeatReceivedTimeMs = 0;

    bool m_connectionAccepted = false;

    std::string m_failReason = "";
};

} // namespace nsf
