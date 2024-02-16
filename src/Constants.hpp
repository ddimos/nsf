#pragma once

namespace nsf
{
    inline constexpr unsigned MAX_PACKET_SIZE_BYTES = 1350;

    inline constexpr int DEFAULT_BIND_PORT_ATTEMPTS_COUNT = 20;

    inline constexpr unsigned DEFAULT_TIME_AFTER_RESEND_RELIABLE_MESSAGE_ms = 200u;
    inline constexpr unsigned DEFAULT_TIME_AFTER_RETRY_CONNECT_ms = 1500u;
    inline constexpr unsigned DEFAULT_TIME_AFTER_SEND_HEARTBEAT_ms = 1000u;
    inline constexpr unsigned DEFAULT_HEARTBEAT_TIMEOUT_ms = 5000u;

} // namespace nsf
