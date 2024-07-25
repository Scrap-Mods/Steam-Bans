---@class luabans
---@field getInterface fun(): ISteamBans Retrieves the LuaBans interface.
luabans = {}

---@enum AccessType
luabans.AccessType =
{
    MIN         = 0,
    Default     = 1,
    Deny        = 2,
    Allow       = 3,
    Ask         = 4,
    MAX         = 5
}

---@class ISteamBans
---@field SetGlobalAccess fun(self: ISteamBans, access: AccessType): nil Sets the global access rights.
---@field GetGlobalAccess fun(self: ISteamBans): AccessType Retrieves the global access rights.
---@field SetUserAccess fun(self: ISteamBans, steamid64: string, access: AccessType): nil Sets the access rights for a specific user.
---@field GetUserAccess fun(self: ISteamBans, steamid64: string): AccessType Retrieves the access rights for a specific user.
---@field SetFriendsAccess fun(self: ISteamBans, access: AccessType): nil Sets the access rights for all of user's friends.
---@field GetFriendsAccess fun(self: ISteamBans): AccessType Retrieves the access rights for all of user's friends.
---@field SetBlockedAccess fun(self: ISteamBans, access: AccessType): nil Sets the access rights for all of user's blocked users.
---@field GetBlockedAccess fun(self: ISteamBans): AccessType Retrieves the access rights for all of user's blocked users.
---@field GetUserAccessList fun(self: ISteamBans): table<string, AccessType> Retrieves the access rights for all users.
---@field SetGlobalAskCallback fun(self: ISteamBans, callback: fun(steamid64: string, old: string, new: string): AccessType): nil Sets the global access callback.
---@field SetUserAskCallback fun(self: ISteamBans, steamid64: string, callback: fun(steamid64: string, old: string, new: string): AccessType): nil Sets the access callback for a specific user.
---@field SetFriendsAskCallback fun(self: ISteamBans, callback: fun(steamid64: string, old: string, new: string): AccessType): nil Sets the access callback for all of user's friends.
---@field SetBlockedAskCallback fun(self: ISteamBans, callback: fun(steamid64: string, old: string, new: string): AccessType): nil Sets the access callback for all of user's blocked users.
---@field GetConnected fun(self: ISteamBans): table<string> Returns a list of all connected users steamids. Only accessible after attaching.
---@field CloseConnection fun(self: ISteamBans, steamid64: string, reason: integer, message: string, linger: boolean): nil Closes a connection to a user.
---@field Attach fun(self: ISteamBans): nil Attaches the interface to Steam.
---@field Detach fun(self: ISteamBans): nil Detaches the interface from Steam.