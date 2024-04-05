#pragma once

#include <cstdint>
#include <functional>
#include <string>

using HSteamNetConnection = std::uint32_t;

class ISteamBans
{
public:
    ISteamBans() {}

    enum class AccessType : std::uint8_t
    {
        Default,
        Deny,
        Allow,
        Ask,
    };

    virtual void SetGlobalAccess(const AccessType& access) = 0;
    virtual AccessType GetGlobalAccess() const = 0;

    virtual void SetUserAccess(const std::uint64_t& steamid, const AccessType& access) = 0;
    virtual AccessType GetUserAcccess(const std::uint64_t& steamid) const = 0;

    virtual void SetGroupAccess(const std::uint64_t& groupid, const AccessType& access) = 0;
    virtual AccessType GetGroupAccess(const std::uint64_t& groupid) const = 0;

    virtual void SetFriendsAccess(const AccessType& access) = 0;
    virtual AccessType GetFriendsAccess() const = 0;

    virtual void SetBlockedAccess(const AccessType& access) = 0;
    virtual AccessType GetBlockedAccess() const = 0;

    virtual const std::unordered_map<std::uint64_t, AccessType>& GetUserAccessList() const = 0;
    virtual const std::unordered_map<std::uint64_t, AccessType>& GetGroupAccessList() const = 0;

    virtual void SetSteamWebApiKey(const std::string& key) = 0;

    virtual void SetGlobalAskCallback(std::function<AccessType(const std::uint64_t&)> callback) = 0;
    virtual void SetUserAskCallback(std::function<AccessType(const std::uint64_t&)> callback) = 0;
    virtual void SetGroupAskCallback(std::function<AccessType(const std::uint64_t&)> callback) = 0;
    virtual void SetFriendsAskCallback(std::function<AccessType(const std::uint64_t&)> callback) = 0;
    virtual void SetBlockedAskCallback(std::function<AccessType(const std::uint64_t&)> callback) = 0;

    virtual bool Attach() = 0;
    virtual bool Detach() = 0;

    virtual std::unordered_map<std::uint64_t, HSteamNetConnection> GetConnections() const = 0;

protected:
    virtual ~ISteamBans() = default;
};

