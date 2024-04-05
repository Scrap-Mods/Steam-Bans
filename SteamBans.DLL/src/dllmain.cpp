#include "ISteamBans.hpp"
#include "windows_include.hpp"

#pragma comment(lib, "SteamBans.SDK.lib")

extern "C" __declspec(dllimport) ISteamBans * CreateSteamBansInterface();

void main(HMODULE hModule)
{
    ISteamBans* pSteamBans = CreateSteamBansInterface();
    if (pSteamBans == NULL)
    {
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }

    if (pSteamBans->Attach())
    {
        pSteamBans->SetGlobalAccess(ISteamBans::AccessType::Deny);

        Sleep(20 * 1000);

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

