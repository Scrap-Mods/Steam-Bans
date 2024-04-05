#include "windows_include.hpp"
#include "CSteamBans.hpp"

// Export SteamBans interface
extern "C" __declspec(dllexport) void* CreateSteamBansInterface()
{
    return SteamBans();
}