#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <map>

#define CON_LOG_PREFIX "[SteamBans] "
#include "Console.h"

//Ignore error 4996
#pragma comment(lib, "steam_api64.lib")
#pragma warning(disable: 4996)
#include <steam/steam_api.h>

struct g_ctx
{
    std::atomic_bool running;
    HANDLE hUnloadEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // Signal to unload
    HANDLE hUnloadEventAck = CreateEvent(NULL, TRUE, FALSE, NULL);
    HMODULE hModule;
    void* vftable_SteamNetConnectionStatusChanged;
    std::vector<unsigned long long> banned_steamids = {};
} Context;

FARPROC GetProcAddressEx(const HMODULE hModule, const std::string lpProcName)
{
    const FARPROC pfnProc = GetProcAddress(hModule, lpProcName.c_str());
    const auto lpProcNameW = std::wstring(lpProcName.begin(), lpProcName.end());
    if (!pfnProc)
    {
        const auto msg = std::format(TEXT("Could not find export {}!"), lpProcNameW);
        MessageBox(NULL, msg.c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
        FreeLibraryAndExitThread(Context.hModule, NULL);
        return nullptr;
    }

    return pfnProc;
}
void onSteamNetConnectionStatusChanged(std::uintptr_t self, SteamNetConnectionStatusChangedCallback_t* pParam);
decltype(&onSteamNetConnectionStatusChanged) oSteamNetConnectionStatusChanged;

bool IsBannedSteamID(const std::uint64_t steamid)
{
    return std::find(Context.banned_steamids.begin(), Context.banned_steamids.end(), steamid) != Context.banned_steamids.end();
}

void onSteamNetConnectionStatusChanged(std::uintptr_t self, SteamNetConnectionStatusChangedCallback_t* pParam)
{
    const std::uint64_t connectionSteamID = pParam->m_info.m_identityRemote.GetSteamID64();
    if (IsBannedSteamID(connectionSteamID))
    {
        if (pParam->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting)
        {
            DebugOutL(std::format(TEXT("Banned SteamID tried to join: {}"), connectionSteamID));
            SteamNetworkingSockets()->CloseConnection(pParam->m_hConn, 0, "You are banned!", false);
        }
        return;
    }

    return oSteamNetConnectionStatusChanged(self, pParam);
}

void UpdateBanList()
{
    // Make or open steamid ban list in the same directory as the process
    std::filesystem::path path = std::filesystem::current_path() / "steam_bans.txt";

    // Create the file if it doesn't exist
    if (!std::filesystem::exists(path))
    {
        std::ofstream file(path);
        file << "# Put your banned steamids here\n";
        file << "# Each steamid should be on a new line\n";
        file << "# For example:\n";
        file << "76561199531536640 # TheGuy920's crash bot\n";
        file.close();
    }

    Context.banned_steamids.clear();
    DebugOutL("Fetching banned users from steam_bans.txt...");

    // Each steamid should be on a new line
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line))
    {
        if (line.size() < 17)
            continue;

        // Ensure 17 numerical digits
        if (!std::all_of(line.begin(), line.begin() + 17, ::isdigit))
            continue;

        // Convert the string to a steamid
        const std::uint64_t steamid64 = std::stoull(line);

        DebugOutL("Added SteamID to banned list: ", steamid64);

        Context.banned_steamids.push_back(steamid64);
    }
    file.close();
}

void SetupChangeNotifications()
{
    HANDLE hNotification = FindFirstChangeNotification(
        std::filesystem::current_path().c_str(),
        FALSE,
        FILE_NOTIFY_CHANGE_LAST_WRITE
    );

    if (hNotification == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, TEXT("Could not setup change notifications!"), TEXT("Error"), MB_OK | MB_ICONERROR);
        FreeLibraryAndExitThread(Context.hModule, NULL);
        return;
    }

    std::filesystem::file_time_type last_write_time_old = std::filesystem::last_write_time(std::filesystem::current_path() / "steam_bans.txt");
    HANDLE handles[2] = { hNotification, Context.hUnloadEvent };

    while (Context.running)
    {
        if (WaitForMultipleObjects(2, handles, FALSE, INFINITE) == WAIT_OBJECT_0)
        {
            // Check if steamid ban list has been updated
            std::filesystem::file_time_type last_write_time = std::filesystem::last_write_time(std::filesystem::current_path() / "steam_bans.txt");
            if (last_write_time != last_write_time_old)
            {
                UpdateBanList();
                last_write_time_old = last_write_time;
            }
        }
        else
        {
            break;
        }
    }

    FindCloseChangeNotification(hNotification);
    SetEvent(Context.hUnloadEventAck);
    FreeLibraryAndExitThread(Context.hModule, NULL);
}

std::uint64_t FollowJMP(std::uintptr_t address)
{
    const std::int32_t offset = *reinterpret_cast<std::int32_t*>(address + 1);
    return address + 5 + offset;
}

using fGetCallbackMap = std::map<std::uint32_t, void*>* (__cdecl*)();

void Main(HMODULE hModule)
{
    Context.running = true;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Console::Attach();


    UpdateBanList();
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetupChangeNotifications, NULL, 0, 0);

    // Check if steam is running
    HMODULE hSteam = GetModuleHandle(TEXT("steam_api64.dll"));
    // Wait until steam is running
    while (!hSteam && Context.running)
    {
        hSteam = GetModuleHandle(TEXT("steam_api64.dll"));
        DebugOutL("Waiting for steam_api64.dll...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    FARPROC pUnregisterCallResult = GetProcAddressEx(hSteam, "SteamAPI_UnregisterCallResult");
    std::uint64_t func = FollowJMP((std::uintptr_t)pUnregisterCallResult);
    func = FollowJMP(func + 0x19);
    fGetCallbackMap GetCallbackMap = (fGetCallbackMap)func;

    std::map<std::uint32_t, void*>* callbacks = GetCallbackMap();

    auto it = callbacks->find(SteamNetConnectionStatusChangedCallback_t::k_iCallback);
    while (it == callbacks->end() && Context.running)
    {
        it = callbacks->find(SteamNetConnectionStatusChangedCallback_t::k_iCallback);
        DebugOutL("Waiting for SteamNetConnectionStatusChangedCallback_t...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    SteamNetConnectionStatusChangedCallback_t* callback = (SteamNetConnectionStatusChangedCallback_t*)it->second;
    if (callback)
    {
        void** vftable = *(void***)callback;
        const int vftable_index = 1;
        Context.vftable_SteamNetConnectionStatusChanged = &vftable[vftable_index];

        DWORD oldProtect;
        VirtualProtect(&vftable[vftable_index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        oSteamNetConnectionStatusChanged = (decltype(oSteamNetConnectionStatusChanged))vftable[vftable_index];
        vftable[vftable_index] = (void*)onSteamNetConnectionStatusChanged;
        VirtualProtect(&vftable[vftable_index], sizeof(void*), oldProtect, &oldProtect);
    }
    else
    {
        DebugOutL(TEXT("Could not find SteamNetConnectionStatusChangedCallback_t!"));
        FreeLibraryAndExitThread(hModule, NULL);
        return;
    }
    DebugOutL("Loaded successfully!");
    DebugOutL(std::format(TEXT("Put your banned steamids in {}"), (std::filesystem::current_path() / "steam_bans.txt").c_str()));
    DebugOutL("Each steamid should be on a new line, and the file should be in the same directory as the process!");
}

BOOL APIENTRY DllMain
(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Main, hModule, 0, 0);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        Context.running = false;
        SignalObjectAndWait(Context.hUnloadEvent, Context.hUnloadEventAck, INFINITE, FALSE);

        if (Context.vftable_SteamNetConnectionStatusChanged && oSteamNetConnectionStatusChanged)
        {
            DWORD oldProtect;
            VirtualProtect(Context.vftable_SteamNetConnectionStatusChanged, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
            *(void**)Context.vftable_SteamNetConnectionStatusChanged = oSteamNetConnectionStatusChanged;
            VirtualProtect(Context.vftable_SteamNetConnectionStatusChanged, sizeof(void*), oldProtect, &oldProtect);
        }

        DebugOutL("Unloaded!");
    }

    return TRUE;
}

