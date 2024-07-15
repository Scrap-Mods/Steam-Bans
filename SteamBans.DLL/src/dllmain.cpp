#include "ISteamBans.hpp"
#include "windows_include.hpp"
#include "steam_include.hpp"

#pragma comment(lib, "SteamBans.SDK.lib")

ISteamBans* pSteamBans = nullptr;

extern "C" __declspec(dllimport) ISteamBans* CreateSteamBansInterface();

void Ban(const std::uint64_t& steamid)
{
    pSteamBans->SetUserAccess(steamid, ISteamBans::AccessType::Deny);
    SteamNetworkingSockets()->CloseConnection(pSteamBans->GetConnections().at(steamid), 0, "Banned", true);
}

void main(HMODULE hModule)
{
    pSteamBans = CreateSteamBansInterface();
    if (pSteamBans == NULL)
    {
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }

    if (pSteamBans->Attach())
    {
        pSteamBans->SetGlobalAccess(ISteamBans::AccessType::Deny);

        const auto& connections = pSteamBans->GetConnections();
        for (const auto& [sid, connection] : connections)
        {
            Ban(sid);
        }

        Sleep(5 * 1000);

        pSteamBans->Detach();
    }

    
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, hModule, 0, NULL);
        break;
    }
    return TRUE;
}

