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

int main(HMODULE hModule)
{
	pSteamBans = CreateSteamBansInterface();
	if (pSteamBans && pSteamBans->Attach())
	{
		pSteamBans->SetGlobalAccess(ISteamBans::AccessType::Deny);

		for (const auto& [sid, connection] : pSteamBans->GetConnections())
			Ban(sid);

		Sleep(5 * 1000);

		pSteamBans->Detach();
	}
	
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, hModule, 0, NULL);

	return TRUE;
}

