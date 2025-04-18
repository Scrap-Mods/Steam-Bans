#include "CSteamBans.hpp"
#include "windows_include.hpp"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

ISteamBans* SteamBans()
{
    static CSteamBans steamBans;
    return &steamBans;
}

CSteamBans::CSteamBans() :
    m_userAccess()
    , m_globalAskCallback(nullptr)
    , m_userAskCallback(nullptr)
    , m_friendsAskCallback(nullptr)
    , m_blockedAskCallback(nullptr)
    , m_callbackMap(nullptr)
    , m_connections(nullptr)
    , m_localSteamID(0)
    , m_connectionMap()
    , m_connStatusChangedHook()
    , m_isAttached(false)
    , m_globalAccess(AccessType::Default)
    , m_friendsAccess(AccessType::Default)
    , m_blockedAccess(AccessType::Default)
    , m_reserved(0)
{
}

void CSteamBans::SetGlobalAccess(const AccessType& access)
{
    m_globalAccess = access;
}

CSteamBans::AccessType CSteamBans::GetGlobalAccess() const
{
    return m_globalAccess;
}

void CSteamBans::SetUserAccess(const std::uint64_t& steamid, const AccessType& access)
{
    m_userAccess[steamid] = access; 
}

CSteamBans::AccessType CSteamBans::GetUserAcccess(const std::uint64_t& steamid) const
{
    return m_userAccess.find(steamid) != m_userAccess.end() ? m_userAccess.at(steamid) : AccessType::Default;
}

void CSteamBans::SetFriendsAccess(const AccessType& access)
{
    m_friendsAccess = access;
}

CSteamBans::AccessType CSteamBans::GetFriendsAccess() const
{
    return m_friendsAccess;
}

void CSteamBans::SetBlockedAccess(const AccessType& access)
{
    m_blockedAccess = access;
}

CSteamBans::AccessType CSteamBans::GetBlockedAccess() const
{
    return m_blockedAccess;
}

const std::unordered_map<std::uint64_t, CSteamBans::AccessType>& CSteamBans::GetUserAccessList() const
{
    return m_userAccess;
}

void CSteamBans::SetGlobalAskCallback(std::function<AccessType(const std::uint64_t&, const ConnState, const ConnState)> callback)
{
    m_globalAskCallback = callback;
}

void CSteamBans::SetUserAskCallback(std::function<AccessType(const std::uint64_t&, const ConnState, const ConnState)> callback)
{
    m_userAskCallback = callback;
}

void CSteamBans::SetFriendsAskCallback(std::function<AccessType(const std::uint64_t&, const ConnState, const ConnState)> callback)
{
    m_friendsAskCallback = callback;
}

void CSteamBans::SetBlockedAskCallback(std::function<AccessType(const std::uint64_t&, const ConnState, const ConnState)> callback)
{
    m_blockedAskCallback = callback;
}

bool CSteamBans::Attach()
{
    if (m_isAttached)
        return false;

    // Ensure steam is loaded
    HMODULE hSteamApi64 = GetModuleHandleA("steam_api64.dll");
    if (hSteamApi64 == NULL)
        return false;

    FARPROC pUnregisterCallResult = GetProcAddress(hSteamApi64, "SteamAPI_UnregisterCallResult");
    if (pUnregisterCallResult == NULL)
        return false;

    // Begin: Hooking SteamNetConnectionStatusChangedCallback_t callback
    std::uintptr_t addrGetCallbackMap = 0;

    try // Switch to __try/__except if doesn't work
    {
        addrGetCallbackMap = follow_jmp(reinterpret_cast<std::uintptr_t>(pUnregisterCallResult));
        addrGetCallbackMap = follow_jmp(addrGetCallbackMap + 0x19);
    }
    catch (...)
    {
        return false;
    }

    pFnGetCallbackMap fnGetCallbackMap = reinterpret_cast<pFnGetCallbackMap>(addrGetCallbackMap);
    auto* pMap = fnGetCallbackMap();
    
    if (pMap == nullptr)
        return false;

    m_callbackMap = pMap;

    auto it = m_callbackMap->find(SteamNetConnectionStatusChangedCallback_t::k_iCallback);
    if (it == m_callbackMap->end())
        return false;

    void* pvftable = *reinterpret_cast<void**>(it->second);
    if (pvftable == nullptr)
        return false;

    m_connStatusChangedHook.Hook(pvftable, 1, CSteamBans::onSteamNetConnectionStatusChanged);
    // End: Hooking SteamNetConnectionStatusChangedCallback_t callback

    // Begin: Hooking g_mapConnections
    std::uintptr_t addrMapConnections = 0;

    try // Switch to __try/__except if doesn't work
    {
        const std::uintptr_t* pvftable2 = *reinterpret_cast<std::uintptr_t***>(SteamNetworkingSockets())[1];
        addrMapConnections = pvftable2[0xE];
        addrMapConnections = follow_jmp(addrMapConnections + 0x2A);
        addrMapConnections = follow_jmp(addrMapConnections + 0x9F + 2);
    }
    catch (...)
    {
        return false;
    }

    std::unordered_map <std::uint64_t, std::uint32_t> g_mapConnections;

    m_connections = reinterpret_cast<CUtlMemory*>(addrMapConnections);
    if (m_connections == nullptr)
        return false;

    UpdateConnections();
    // End: Hooking g_mapConnections

    m_isAttached = true;

    return true;
}

bool CSteamBans::IsAttached()
{
    return m_isAttached;
}

bool CSteamBans::Detach()
{
    if (!m_isAttached)
        return false;

    m_isAttached = false;

    m_connStatusChangedHook.Unhook();
    return true;
}

const std::unordered_map<std::uint64_t, HSteamNetConnection>& CSteamBans::GetConnections() const
{
    // Get active connections
    return m_connectionMap;
}

void CSteamBans::UpdateConnections()
{
    if (m_connections == nullptr)
        return;

    m_connectionMap.clear();

    for (std::int32_t i = 0; i < m_connections->m_nCapacity; i++)
    {
        if (m_connections->m_pData[i].m_pElement == nullptr)
            continue;

        const std::uint64_t remoteSteamID = m_connections->m_pData[i].m_pElement->m_identityRemote;
        const std::uint64_t localSteamID = m_connections->m_pData[i].m_pElement->m_identityLocal;
        const std::uint32_t connection = m_connections->m_pData[i].m_pElement->m_hConnectionSelf;

        if (m_localSteamID == 0)
            m_localSteamID = localSteamID;

        if (localSteamID == remoteSteamID)
            continue;

        m_connectionMap[remoteSteamID] = connection;
    }
}

std::uintptr_t CSteamBans::follow_jmp(const std::uintptr_t& address) const
{
    const std::int32_t offset = *reinterpret_cast<std::int32_t*>(address + 1);
    return address + 5 + offset;
}

// Main logic for preventing connections
void CSteamBans::onSteamNetConnectionStatusChanged(std::uintptr_t self, SteamNetConnectionStatusChangedCallback_t* pParam)
{
    CSteamBans* steamBans = reinterpret_cast<CSteamBans*>(SteamBans());
    steamBans->UpdateConnections();
    const std::uint64_t connectionSteamID = pParam->m_info.m_identityRemote.GetSteamID64();
    
    // Handle user access
    AccessType access = steamBans->GetUserAcccess(connectionSteamID);
    if (access != AccessType::Default)
    {
        switch (access)
        {
            case AccessType::Deny:
            {
                SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
                return;
            }
            case AccessType::Allow:
            {
                return steamBans->m_connStatusChangedHook.GetOriginalFunction()(self, pParam);
            }
            case AccessType::Ask:
            {
                if (steamBans->m_userAskCallback != nullptr)
                {
                    access = steamBans->m_userAskCallback(connectionSteamID, ConnState(pParam->m_eOldState), ConnState(pParam->m_info.m_eState));
                }
                else
                {
                    access = AccessType::Allow;
                }

                if (access == AccessType::Deny)
                {
                    SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
                    return;
                }
                else
                {
                    return steamBans->m_connStatusChangedHook.GetOriginalFunction()(self, pParam);
                }
            }
            case AccessType::Default: {}
        }
    }

    // Handle friends access
    const EFriendRelationship relationship = SteamFriends()->GetFriendRelationship(connectionSteamID);
    access = steamBans->GetFriendsAccess();
    if (access != AccessType::Default && relationship == k_EFriendRelationshipFriend)
    {
        switch (access)
        {
            case AccessType::Deny:
            {
                SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
                return;
            }
            case AccessType::Allow:
            {
                return steamBans->m_connStatusChangedHook.GetOriginalFunction()(self, pParam);
            }
            case AccessType::Ask:
            {
                if (steamBans->m_friendsAskCallback != nullptr)
                {
                    access = steamBans->m_friendsAskCallback(connectionSteamID, ConnState(pParam->m_eOldState), ConnState(pParam->m_info.m_eState));
                }
                else
                {
                    access = AccessType::Allow;
                }

                if (access == AccessType::Deny)
                {
                    SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
                    return;
                }
                else
                {
                    return steamBans->m_connStatusChangedHook.GetOriginalFunction()(self, pParam);
                }
            }
            case AccessType::Default: {}
        }
    }

    // Handle blocked access
    access = steamBans->GetBlockedAccess();
    const bool isBlocked = relationship == k_EFriendRelationshipBlocked || relationship == k_EFriendRelationshipIgnored;
    if (access != AccessType::Default && isBlocked)
    {
        switch (access)
        {
            case AccessType::Deny:
            {
                SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
                return;
            }
            case AccessType::Allow:
            {
                return steamBans->m_connStatusChangedHook.GetOriginalFunction()(self, pParam);
            }
            case AccessType::Ask:
            {
                if (steamBans->m_blockedAskCallback != nullptr)
                {
                    access = steamBans->m_blockedAskCallback(connectionSteamID, ConnState(pParam->m_eOldState), ConnState(pParam->m_info.m_eState));
                }
                else
                {
                    access = AccessType::Allow;
                }

                if (access == AccessType::Deny)
                {
                    SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
                    return;
                }
                else
                {
                    return steamBans->m_connStatusChangedHook.GetOriginalFunction()(self, pParam);
                }
            }
            case AccessType::Default: {}
        }
    }

    // Handle global access
    access = steamBans->GetGlobalAccess();
    if (access != AccessType::Default)
    {
        switch (access)
        {
        case AccessType::Deny:
        {
            SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
            return;
        }
        case AccessType::Allow:
        {
            return steamBans->m_connStatusChangedHook.GetOriginalFunction()(self, pParam);
        }
        case AccessType::Ask:
        {
            if (steamBans->m_globalAskCallback != nullptr)
            {
                access = steamBans->m_globalAskCallback(connectionSteamID, ConnState(pParam->m_eOldState), ConnState(pParam->m_info.m_eState));
            }
            else
            {
                access = AccessType::Allow;
            }

            if (access == AccessType::Deny)
            {
                SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, nullptr, false);
                return;
            }
            else
            {
                return steamBans->m_connStatusChangedHook.GetOriginalFunction()(self, pParam);
            }
        }
        case AccessType::Default: {}
        }
    }


    // Allow connection by default
    return steamBans->m_connStatusChangedHook.GetOriginalFunction()(self, pParam);
}