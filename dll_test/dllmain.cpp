// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include "function.h"
#include "Proc.h"

constexpr int FRAME_SIZE = 38 * 39;
constexpr int REFRESH_RATE = 500;

UpdateData_t     o_UpdateData;
IsServer_t       o_IsServer;
SetRefreshRate_t SetRefreshRate;
SetBlockData_t   SetBlockData;
GetBlockColors_t GetBlockColors;

bool __fastcall IsServer(void* ret) {
    g_core = ret;
    return o_IsServer(ret);
}


int64_t __fastcall UpdateData(void* ret) {
    int64_t return_data = o_UpdateData(ret);
    UINT a4, a5;
    int deep = 75;
    wchar_t w[] = L"白癡";
    for (int i = 0; i < FRAME_SIZE; i++) {
        //swprintf_s(w, L"%d%%", deep);
        GetBlockColors(ret, deep, &a4, &a5);
        SetBlockData(ret, i, w, a4, a5);
    }
    return return_data;
}

DWORD WINAPI attach(LPVOID) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    EnumWindows(EnumWindowsProc, GetCurrentProcessId());
    g_oWndProc = (WndProc_t)GetWindowLongPtr(g_HWND, GWLP_WNDPROC);
    SetWindowLongPtr(g_HWND, GWLP_WNDPROC, (LONG_PTR)WndProc);
    MODULEINFO module_info = get_module_info("taskmgr.exe");

    g_base_address    =           (ULONG64)module_info.lpBaseOfDll;
    g_RefreshRate_ptr =  *address_offset<ULONG64>(g_base_address + 0x11C830, 0x110);

    o_UpdateData      = (UpdateData_t)    (find_pattern(&module_info, patten.UpdateData) + patten.UpdateData_offset);
    o_IsServer        = (IsServer_t)      (find_pattern(&module_info, patten.IsServer) + patten.IsServer_offset);
    SetRefreshRate    = (SetRefreshRate_t)(find_pattern(&module_info, patten.SetRefreshRate) + patten.SetRefreshRate_offset);
    GetBlockColors    = (GetBlockColors_t)(find_pattern(&module_info, patten.GetBlockColors) + patten.GetBlockColors_offset);
    SetBlockData      = (SetBlockData_t)  (find_pattern(&module_info, patten.SetBlockData) + patten.SetBlockData_offset);

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&o_UpdateData, UpdateData);
    DetourAttach((PVOID*)&o_IsServer, IsServer);
    DetourTransactionCommit();
    
    SetRefreshRate(g_RefreshRate_ptr, 60);
    while (!g_core) {
        Sleep(500);
    }

    UINT16* cpu_count = (UINT16*)((BYTE*)g_core + 0x944);
    std::cout << g_core << "," << *cpu_count << std::endl;

    Sleep(500);
    *cpu_count = FRAME_SIZE;
    std::cout << *cpu_count << std::endl;

    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        DWORD dwThreadId;
        g_hInstance = hModule;
        CreateThread(NULL, 0, attach, hModule, 0, &dwThreadId);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH: {
        //MessageBoxW(NULL, L"結束掛勾", 0, 0);
        SetRefreshRate(g_RefreshRate_ptr, 500);
        break;
    }
    }
    return TRUE;
}

/*
    o_UpdateData      =      (UpdateData_t)(g_base_address + 0xC9CC8);
    o_IsServer        =        (IsServer_t)(g_base_address + 0x195BC);
    SetRefreshRate    =  (SetRefreshRate_t)(g_base_address + 0x6063C);
    GetBlockColors    =  (GetBlockColors_t)(g_base_address + 0xC9158);
    SetBlockData      =    (SetBlockData_t)(g_base_address + 0xC9B70);
 */