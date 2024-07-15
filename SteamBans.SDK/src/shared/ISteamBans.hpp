#pragma once

#include <cstdint>
#include <functional>
#include <string>


enum class _ESteamNetworkingConnectionState
{
	k_ESteamNetworkingConnectionState_None = 0,
	k_ESteamNetworkingConnectionState_Connecting = 1,
	k_ESteamNetworkingConnectionState_FindingRoute = 2,
	k_ESteamNetworkingConnectionState_Connected = 3,
	k_ESteamNetworkingConnectionState_ClosedByPeer = 4,
	k_ESteamNetworkingConnectionState_ProblemDetectedLocally = 5,
	k_ESteamNetworkingConnectionState_FinWait = -1,
	k_ESteamNetworkingConnectionState_Linger = -2,
	k_ESteamNetworkingConnectionState_Dead = -3,
	k_ESteamNetworkingConnectionState__Force32Bit = 0x7fffffff
};
using ConnState = _ESteamNetworkingConnectionState;

using HSteamNetConnection = std::uint32_t;

class ISteamBans
{
public:
    ISteamBans() {}

    enum class AccessType : std::uint8_t
    {
        MIN,
        Default,
        Deny,
        Allow,
        Ask,
        MAX,
    };

    virtual void SetGlobalAccess(const AccessType& access) = 0;
    virtual AccessType GetGlobalAccess() const = 0;

    virtual void SetUserAccess(const std::uint64_t& steamid, const AccessType& access) = 0;
    virtual AccessType GetUserAcccess(const std::uint64_t& steamid) const = 0;

    virtual void SetFriendsAccess(const AccessType& access) = 0;
    virtual AccessType GetFriendsAccess() const = 0;

    virtual void SetBlockedAccess(const AccessType& access) = 0;
    virtual AccessType GetBlockedAccess() const = 0;

    virtual const std::unordered_map<std::uint64_t, AccessType>& GetUserAccessList() const = 0;

    virtual void SetGlobalAskCallback(std::function<AccessType(const std::uint64_t&, const ConnState, const ConnState)> callback) = 0;
    virtual void SetUserAskCallback(std::function<AccessType(const std::uint64_t&, const ConnState, const ConnState)> callback) = 0;
    virtual void SetFriendsAskCallback(std::function<AccessType(const std::uint64_t&, const ConnState, const ConnState)> callback) = 0;
    virtual void SetBlockedAskCallback(std::function<AccessType(const std::uint64_t&, const ConnState, const ConnState)> callback) = 0;

    virtual bool Attach() = 0;
    virtual bool Detach() = 0;

    virtual const std::unordered_map<std::uint64_t, HSteamNetConnection>& GetConnections() const = 0;

protected:
    virtual ~ISteamBans() = default;
};

