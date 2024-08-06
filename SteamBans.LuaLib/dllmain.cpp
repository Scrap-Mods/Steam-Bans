#include "ISteamBans.hpp"
#include "windows_include.hpp"
#include "steam_include.hpp"

#pragma comment(lib, "SteamBans.SDK.lib")
#include <luajit/lua.hpp>

#include <algorithm>
#include <cassert>

extern "C" __declspec(dllimport) ISteamBans* CreateSteamBansInterface();
using namespace std::placeholders;

namespace luabans
{
    ISteamBans* luabans_checkisteambans(lua_State* L, const int argnum) {
        ISteamBans** ppSteamBans = reinterpret_cast<ISteamBans**>(luaL_checkudata(L, argnum, "ISteamBans"));

        if (ppSteamBans == nullptr)
        {
            luaL_argerror(L, argnum, "Invalid ISteamBans");
            return nullptr;
        }

        return *ppSteamBans;
    }

    ISteamBans::AccessType luabans_checkaccesstype(lua_State* L, const int argnum) {
        ISteamBans::AccessType access = ISteamBans::AccessType(luaL_checkinteger(L, argnum));

        if (access <= ISteamBans::AccessType::MIN || access > ISteamBans::AccessType::MAX)
            luaL_argerror(L, argnum, "Invalid AccessType");

        return access;
    }

    std::uint64_t luabans_checksteamid64(lua_State* L, const int argnum) {
        size_t len;
        const char* str = luaL_checklstring(L, argnum, &len);
        std::string steamid(str, len);

        if (steamid.size() < 17)
            luaL_argerror(L, argnum, "steamid64 expected length 17");

        // Ensure 17 numerical digits
        if (!std::all_of(steamid.begin(), steamid.begin() + 17, ::isdigit))
            luaL_argerror(L, argnum, "steamid64 must only contain digits");

        // Convert the string to a steamid
        const std::uint64_t steamid64 = std::stoull(steamid);

        return steamid64;
    }

    void luabans_checkarglen(lua_State* L, const int expectedargs)
    {
        const int args = lua_gettop(L);
        if (args != expectedargs)
            luaL_error(L, "Expected %d args but got %d", expectedargs, args);
    }

    // void luabans:SetGlobalAccess(steambans.AccessType.Default)
    int luabans_SetGlobalAccess(lua_State* L) {
        luabans_checkarglen(L, 2);

        lua_pushvalue(L, 1);
        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        ISteamBans::AccessType access = luabans_checkaccesstype(L, 2);

        pSteamBans->SetGlobalAccess(access);
        return 1;
    }

    // luabans.AccessType steambans:GetGlobalAccess()
    int luabans_GetGlobalAccess(lua_State* L) {
        luabans_checkarglen(L, 1);

        lua_pushvalue(L, 1);
        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);

        lua_pushinteger(L, (lua_Integer)pSteamBans->GetGlobalAccess());
        return 1;
    }

    // void steambans:SetUserAccess()
    int luabans_SetUserAccess(lua_State* L) {
        luabans_checkarglen(L, 3);

        lua_pushvalue(L, 1);
        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        const std::uint64_t steamid = luabans_checksteamid64(L, 2);
        ISteamBans::AccessType access = luabans_checkaccesstype(L, 3);

        pSteamBans->SetUserAccess(steamid, access);
        return 1;
    }

    // luabans.AccessType steambans:GetUserAccess()
    int luabans_GetUserAccess(lua_State* L) {
        luabans_checkarglen(L, 2);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        const std::uint64_t steamid = luabans_checksteamid64(L, 2);
        
        lua_pushinteger(L, (lua_Integer)pSteamBans->GetUserAcccess(steamid));
        return 1;
    }

    // void steambans:SetFriendsAccess()
    int luabans_SetFriendsAccess(lua_State* L) {
        luabans_checkarglen(L, 2);

        lua_pushvalue(L, 1);
        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        ISteamBans::AccessType access = luabans_checkaccesstype(L, 2);

        pSteamBans->SetFriendsAccess(access);
        return 1;
    }

    // luabans.AccessType steambans:GetFriendsAccess()
    int luabans_GetFriendsAccess(lua_State* L) {
        luabans_checkarglen(L, 1);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);

        lua_pushinteger(L, (lua_Integer)pSteamBans->GetFriendsAccess());
        return 1;
    }

    // void steambans:SetBlockedAccess()
    int luabans_SetBlockedAccess(lua_State* L) {
        luabans_checkarglen(L, 2);

        lua_pushvalue(L, 1);
        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        ISteamBans::AccessType access = luabans_checkaccesstype(L, 2);

        pSteamBans->SetBlockedAccess(access);
        return 1;
    }

    // luabans.AccessType steambans:GetBlockedAccess()
    int luabans_GetBlockedAccess(lua_State* L) {
        luabans_checkarglen(L, 1);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);

        lua_pushinteger(L, (lua_Integer)pSteamBans->GetBlockedAccess());
        return 1;
    }

    // table<steamid, accesstype> steambans:GetUserAccessList()
    int luabans_GetUserAccessList(lua_State* L) {
        luabans_checkarglen(L, 1);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);

        auto& accessList = pSteamBans->GetUserAccessList();

        lua_createtable(L, 0, accessList.size());
        for (const auto& [steamid, access] : accessList)
        {
            lua_pushstring(L, std::to_string(steamid).c_str());
            lua_pushinteger(L, (lua_Integer)access);
            lua_rawset(L, -3);
        }
        
        return 1;
    }

    /*enum class _ESteamNetworkingConnectionState
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
    };*/

    const char* ConnState_ToString(const ConnState state)
    {
        switch (state)
        {
        case ConnState::k_ESteamNetworkingConnectionState_None:
            return "None";
        case ConnState::k_ESteamNetworkingConnectionState_Connecting:
            return "Connecting";
        case ConnState::k_ESteamNetworkingConnectionState_FindingRoute:
            return "FindingRoute";
        case ConnState::k_ESteamNetworkingConnectionState_Connected:
            return "Connected";
        case ConnState::k_ESteamNetworkingConnectionState_ClosedByPeer:
            return "ClosedByPeer";
        case ConnState::k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
            return "ProblemDetectedLocally";
        case ConnState::k_ESteamNetworkingConnectionState_FinWait:
            return "FinWait";
        case ConnState::k_ESteamNetworkingConnectionState_Linger:
            return "Linger";
        case ConnState::k_ESteamNetworkingConnectionState_Dead:
            return "Dead";
        case ConnState::k_ESteamNetworkingConnectionState__Force32Bit:
            return "_Force32Bit";
        default:
            return "Unknown";
        }
    }

    ISteamBans::AccessType DoAskCallback(lua_State* L, const char* callback, const std::uint64_t steamid, const ConnState oldState, const ConnState newState)
    {
        lua_getfield(L, LUA_REGISTRYINDEX, "steambans_callbacks");
        lua_getfield(L, -1, callback);

        if (lua_type(L, -1) == LUA_TFUNCTION)
        {
            lua_pushstring(L, std::to_string(steamid).c_str());
            lua_pushstring(L, ConnState_ToString(oldState));
            lua_pushstring(L, ConnState_ToString(newState));
            lua_call(L, 3, 1);
            ISteamBans::AccessType access = luabans_checkaccesstype(L, -1);

            lua_pop(L, 2);

            return access;
        }

        lua_pop(L, 3);

        return ISteamBans::AccessType::Allow;
    }

    // void steambans:SetGlobalAskCallback()
    int luabans_SetGlobalAskCallback(lua_State* L) {
        luabans_checkarglen(L, 2);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);

        lua_getfield(L, LUA_REGISTRYINDEX, "steambans_callbacks");
        lua_pushvalue(L, 2);
        lua_setfield(L, -2, "global");

        pSteamBans->SetGlobalAskCallback(std::bind(DoAskCallback, L, "global", _1, _2, _3));

        lua_pop(L, 1);
        lua_pushnil(L);

        return 1;
    }

    // void steambans:SetUserAskCallback()
    int luabans_SetUserAskCallback(lua_State* L) {
        luabans_checkarglen(L, 2);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);

        lua_getfield(L, LUA_REGISTRYINDEX, "steambans_callbacks");
        lua_pushvalue(L, 2);
        lua_setfield(L, -2, "user");

        pSteamBans->SetGlobalAskCallback(std::bind(DoAskCallback, L, "user", _1, _2, _3));

        lua_pop(L, 1);
        lua_pushnil(L);

        return 1;
    }

    // void steambans:SetFriendsAskCallback()
    int luabans_SetFriendsAskCallback(lua_State* L) {
        luabans_checkarglen(L, 2);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);

        lua_getfield(L, LUA_REGISTRYINDEX, "steambans_callbacks");
        lua_pushvalue(L, 2);
        lua_setfield(L, -2, "friends");

        pSteamBans->SetGlobalAskCallback(std::bind(DoAskCallback, L, "friends", _1, _2, _3));

        lua_pop(L, 1);
        lua_pushnil(L);

        return 1;
    }

    // void steambans:SetBlockedAskCallback()
    int luabans_SetBlockedAskCallback(lua_State* L) {
        luabans_checkarglen(L, 2);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);

        lua_getfield(L, LUA_REGISTRYINDEX, "steambans_callbacks");
        lua_pushvalue(L, 2);
        lua_setfield(L, -2, "blocked");

        pSteamBans->SetGlobalAskCallback(std::bind(DoAskCallback, L, "blocked", _1, _2, _3));

        lua_pop(L, 1);
        lua_pushnil(L);

        return 1;
    }

    // table<steamid, lightuserdata> steambans:GetConnections()
    int luabans_GetConnected(lua_State* L) {
        luabans_checkarglen(L, 1);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);

        auto& connections = pSteamBans->GetConnections();

        lua_createtable(L, connections.size(), 0);
        int idx = 1;
        for (const auto& [steamid, connection] : connections)
        {
            lua_pushinteger(L, idx++);
            lua_pushstring(L, std::to_string(steamid).c_str());
            lua_rawset(L, -3);
        }

        return 1;
    }

    int luabans_CloseConnection(lua_State* L) {
        luabans_checkarglen(L, 5);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        const std::uint64_t steamid = luabans_checksteamid64(L, 2);
        const int reason = luaL_checkinteger(L, 3);
        const char* msg = luaL_checklstring(L, 4, 0);
        if (!lua_isboolean(L, 5))
            luaL_argerror(L, 5, "boolean expected");
        const bool linger = lua_toboolean(L, 5);

        auto& connections = pSteamBans->GetConnections();
        if (auto connection = connections.find(steamid); connection != connections.end())
            SteamNetworkingSockets()->CloseConnection(connection->second, reason, msg, linger);

        lua_pushnil(L);
        return 1;
    }

    int luabans_SendMessageToConnection(lua_State* L) {
        luabans_checkarglen(L, 4);

        const ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        const std::uint64_t steamid = luabans_checksteamid64(L, 2);
        if (!lua_istable(L, 3)) luaL_argerror(L, 3, "table expected");
        const int sendflags = luaL_checkinteger(L, 4);

        const std::size_t tableLen = lua_objlen(L, 3);
        std::vector<std::uint8_t> packetData(tableLen);
        for (int i = 1; i <= tableLen; i++)
        {
            lua_rawgeti(L, 3, i);
            if (lua_type(L, -1) != LUA_TNUMBER)
            {
                lua_pop(L, 1);
                lua_pushboolean(L, 0);
                return 1;
            }

            packetData[i-1] = (std::uint8_t)lua_tointeger(L, -1);
            lua_pop(L, 1);
        }

        const auto& connections = pSteamBans->GetConnections();
        auto connection = connections.find(steamid);
        if (connection == connections.end())
        {
            lua_pushboolean(L, 0);
            return 1;
        }
        
        SteamNetworkingSockets()->SendMessageToConnection(connection->second, packetData.data(), packetData.size(), sendflags, nullptr);
        lua_pushboolean(L, 1);
        return 1;
    }

    // bool steambans:Attach()
    int luabans_Attach(lua_State* L) {
        luabans_checkarglen(L, 1);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        lua_pushboolean(L, pSteamBans->Attach());
        
        lua_pushvalue(L, 1);
        lua_setfield(L, LUA_REGISTRYINDEX, "luabans_interface");

        return 1;
    }

    // bool steambans:Detach()
    int luabans_Detach(lua_State* L) {
        luabans_checkarglen(L, 1);

        ISteamBans* pSteamBans = luabans_checkisteambans(L, 1);
        lua_pushboolean(L, pSteamBans->Detach());

        lua_pushnil(L);
        lua_setfield(L, LUA_REGISTRYINDEX, "luabans_interface");

        return 1;
    }

    const luaL_Reg MetaISteamBans[] = {
        { "SetGlobalAccess", luabans_SetGlobalAccess },
        { "GetGlobalAccess", luabans_GetGlobalAccess },
        { "SetUserAccess", luabans_SetUserAccess },
        { "GetUserAccess", luabans_GetUserAccess },
        { "SetFriendsAccess", luabans_SetFriendsAccess },
        { "GetFriendsAccess", luabans_GetFriendsAccess },
        { "SetBlockedAccess", luabans_SetBlockedAccess },
        { "GetBlockedAccess", luabans_GetBlockedAccess },
        { "GetUserAccessList", luabans_GetUserAccessList },
        { "SetGlobalAskCallback", luabans_SetGlobalAskCallback },
        { "SetUserAskCallback", luabans_SetUserAskCallback },
        { "SetFriendsAskCallback", luabans_SetFriendsAskCallback },
        { "SetBlockedAskCallback", luabans_SetBlockedAskCallback },
        { "GetConnected", luabans_GetConnected },
        { "CloseConnection", luabans_CloseConnection },
        { "SendMessageToConnection", luabans_SendMessageToConnection },
        { "Attach", luabans_Attach },
        { "Detach", luabans_Detach },
        {NULL, NULL}
    };

    int luabans_getInterface(lua_State* L) {
        

        ISteamBans** ppSteamBans = (ISteamBans**)lua_newuserdata(L, 8);
        *ppSteamBans = CreateSteamBansInterface();

        luaL_newmetatable(L, "ISteamBans");

        lua_createtable(L, 0, 18);
        luaL_register(L, "", &MetaISteamBans[0]);
        lua_setfield(L, -2, "__index");

        lua_pushcclosure(L, luabans_Detach, 0);
        lua_setfield(L, -2, "__gc");

        lua_setmetatable(L, -2);

        return 1;
    }
};


static const struct luaL_Reg functions[] = {
    {"getInterface", luabans::luabans_getInterface},
    {NULL, NULL}
};


extern "C" {
    __declspec(dllexport) int luaopen_luabans(lua_State* L) {
        luaL_register(L, "luabans", functions);

        // Register AccessType enum
        lua_createtable(L, 4, 4);

        lua_pushinteger(L, (int)ISteamBans::AccessType::Default);
        lua_setfield(L, -2, "Default");
        lua_pushinteger(L, (int)ISteamBans::AccessType::Deny);
        lua_setfield(L, -2, "Deny");
        lua_pushinteger(L, (int)ISteamBans::AccessType::Allow);
        lua_setfield(L, -2, "Allow");
        lua_pushinteger(L, (int)ISteamBans::AccessType::Ask);
        lua_setfield(L, -2, "Ask");

        lua_pushinteger(L, (int)ISteamBans::AccessType::Default);
        lua_pushliteral(L, "Default");
        lua_rawset(L, -3);
        lua_pushinteger(L, (int)ISteamBans::AccessType::Deny);
        lua_pushliteral(L, "Deny");
        lua_rawset(L, -3);
        lua_pushinteger(L, (int)ISteamBans::AccessType::Allow);
        lua_pushliteral(L, "Allow");
        lua_rawset(L, -3);
        lua_pushinteger(L, (int)ISteamBans::AccessType::Ask);
        lua_pushliteral(L, "Ask");
        lua_rawset(L, -3);

        lua_setfield(L, -2, "AccessType");

        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, "steambans_callbacks");

        return 1;
    }
}