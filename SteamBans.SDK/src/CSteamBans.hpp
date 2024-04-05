#pragma once
#pragma warning( disable : 4711 4710 ) // Disable auto inlining warnings

#include "shared/ISteamBans.hpp"
#include "VFTableHook.hpp"
#include "steam_include.hpp"

#include <map>
#include <string>

class CSteamBans : public ISteamBans
{
public:
    CSteamBans();
    ~CSteamBans() = default;

    void SetGlobalAccess(const AccessType& access) override;
    AccessType GetGlobalAccess() const override;

    void SetUserAccess(const std::uint64_t& steamid, const AccessType& access) override;
    AccessType GetUserAcccess(const std::uint64_t& steamid) const override;

    void SetGroupAccess(const std::uint64_t& groupid, const AccessType& access) override;
    AccessType GetGroupAccess(const std::uint64_t& groupid) const override;

    void SetFriendsAccess(const AccessType& access) override;
    AccessType GetFriendsAccess() const override;

    void SetBlockedAccess(const AccessType& access) override;
    AccessType GetBlockedAccess() const override;

    const std::unordered_map<std::uint64_t, AccessType>& GetUserAccessList() const override;
    const std::unordered_map<std::uint64_t, AccessType>& GetGroupAccessList() const override;

    void SetSteamWebApiKey(const std::string& key) override;

    void SetGlobalAskCallback(std::function<AccessType(const std::uint64_t&)> callback) override;
    void SetUserAskCallback(std::function<AccessType(const std::uint64_t&)> callback) override;
    void SetGroupAskCallback(std::function<AccessType(const std::uint64_t&)> callback) override;
    void SetFriendsAskCallback(std::function<AccessType(const std::uint64_t&)> callback) override;
    void SetBlockedAskCallback(std::function<AccessType(const std::uint64_t&)> callback) override;

    bool Attach() override;
    bool Detach() override;

    std::unordered_map<std::uint64_t, HSteamNetConnection> GetConnections() const override;
private:

    std::string m_steamWebApiKey;

    std::unordered_map<std::uint64_t, AccessType> m_userAccess;
    std::unordered_map<std::uint64_t, AccessType> m_groupAccess;

    std::function<AccessType(const std::uint64_t&)> m_globalAskCallback;
    std::function<AccessType(const std::uint64_t&)> m_userAskCallback;
    std::function<AccessType(const std::uint64_t&)> m_groupAskCallback;
    std::function<AccessType(const std::uint64_t&)> m_friendsAskCallback;
    std::function<AccessType(const std::uint64_t&)> m_blockedAskCallback;

    std::uintptr_t follow_jmp(const std::uintptr_t& address) const;

    using pFnGetCallbackMap = std::map<std::uint32_t, void*>*(*)(void);
    const std::map<std::uint32_t, void*>* m_callbackMap;

    static void onSteamNetConnectionStatusChanged(std::uintptr_t self, SteamNetConnectionStatusChangedCallback_t* pParam);
    VFTableHook<decltype(onSteamNetConnectionStatusChanged)*> m_connStatusChangedHook;

    bool m_isAttached;
    AccessType m_globalAccess;
    AccessType m_friendsAccess;
    AccessType m_blockedAccess;

    std::uint32_t m_reserved;
};

ISteamBans* SteamBans();