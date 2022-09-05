// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include "function.h"
#include "Proc.h"

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
    wchar_t text[5];
    
    for (int i = 0; i < FRAME_SIZE; i++) {
        UINT value = 100 - (float)o_data_pack->pixel[i] / 255.0 * 100;
        swprintf_s(text, L"%d%%", value);
        GetBlockColors(ret, value, &a4, &a5);
        SetBlockData(ret, i, text, a4, a5);
    }

    o_data_pack->frame_done = TRUE; // 代表更新

    return return_data;
}

DWORD WINAPI attach(LPVOID) {
    //通訊

    EnumWindows(EnumWindowsProc, GetCurrentProcessId());
    g_oWndProc = (WndProc_t)GetWindowLongPtr(g_HWND, GWLP_WNDPROC);
    SetWindowLongPtr(g_HWND, GWLP_WNDPROC, (LONG_PTR)WndProc);
    MODULEINFO module_info = get_module_info("Taskmgr.exe");

    g_base_address    = (ULONG64)module_info.lpBaseOfDll;
    g_RefreshRate_ptr = *address_offset<ULONG64>(g_base_address + 0x11C830, 0x110);

    o_UpdateData      = (UpdateData_t)    (find_pattern(&module_info, g_patten.UpdateData) + g_patten.UpdateData_offset);
    o_IsServer        = (IsServer_t)      (find_pattern(&module_info, g_patten.IsServer) + g_patten.IsServer_offset);
    SetRefreshRate    = (SetRefreshRate_t)(find_pattern(&module_info, g_patten.SetRefreshRate) + g_patten.SetRefreshRate_offset);
    GetBlockColors    = (GetBlockColors_t)(find_pattern(&module_info, g_patten.GetBlockColors) + g_patten.GetBlockColors_offset);
    SetBlockData      = (SetBlockData_t)  (find_pattern(&module_info, g_patten.SetBlockData) + g_patten.SetBlockData_offset);

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&o_UpdateData, UpdateData);
    DetourAttach((PVOID*)&o_IsServer, IsServer);
    DetourTransactionCommit();
    
    SetRefreshRate(g_RefreshRate_ptr, REFRESH_RATE);
    while (!g_core) {
        Sleep(500);
    }

    UINT16* cpu_count = (UINT16*)((BYTE*)g_core + 0x944);
    Sleep(500);

    std::cout << u8"注入成功，可以使用了, core=" << *cpu_count << std::endl;
    *cpu_count = FRAME_SIZE;
    return true;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        SetConsoleOutputCP(CP_UTF8);

        HANDLE hFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, L"Global\\dllmemfilemap123");
        o_data_pack = (DataPack*)MapViewOfFile(hFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        DWORD dwThreadId;
        g_hInstance = hModule;
        CreateThread(NULL, 0, attach, hModule, 0, &dwThreadId);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH: {
        SetRefreshRate(g_RefreshRate_ptr, 500);
        //MessageBoxW(NULL, L"結束掛勾", 0, 0);
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