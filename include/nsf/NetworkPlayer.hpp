#pragma once 
#include <string>
#include "nsf/Types.hpp"

namespace nsf
{

class NetworkPlayer
{
public:
    NetworkPlayer() = default;
    NetworkPlayer(const std::string& _name, PlayerID _id, bool _isLocal)
    : m_name(_name), m_id(_id), m_isLocalPlayer(_isLocal)
    {
        if (m_name.empty())
            m_name = "Player";
    }

    const std::string& getName() const { return m_name; }
    PlayerID getPlayerId() const { return m_id; }
    PeerID getPeerId() const { return m_peerId; } // TODO it's available only on a host side
    bool isLocal() const { return m_isLocalPlayer; }

    bool isLeft() const { return m_isLeft; }
    void onLeft() { m_isLeft = true; }

    void setPlayerId(PlayerID _playerId) { m_id = _playerId; }
    void setPeerId(PeerID _peerId) { m_peerId = _peerId; }

private:
    std::string m_name = "Player";
    PlayerID m_id = PlayerIdInvalid;
    bool m_isLocalPlayer = false;

    PeerID  m_peerId = PeerIdInvalid;

    bool m_isLeft = false;
};

} // namespace nsf
