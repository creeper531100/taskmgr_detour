// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include "function.h"
#include "Proc.h"

UpdateData_t     o_UpdateData;
IsServer_t       o_IsServer;
SetRefreshRate_t SetRefreshRate;
SetBlockData_t   SetBlockData;
GetBlockColors_t GetBlockColors;

bool __fastcall IsServer(void* self) {
    g_core = self;
    return o_IsServer(self);
}

int64_t __fastcall UpdateData(void* self) {
    int64_t ret = o_UpdateData(self);
    UINT a4, a5;
    wchar_t text[5];
    
    for (int i = 0; i < o_data_pack->frame_size; i++) {
        UINT value = 100 - (float)o_data_pack->pixel[i] / 255.0 * 100;
        swprintf_s(text, L"%d%%", value);
        GetBlockColors(self, value, &a4, &a5);
        SetBlockData(self, i, text, a4, a5);
    }

    o_data_pack->frame_done = TRUE; // 代表更新

    return ret;
}

DWORD WINAPI attach(LPVOID) {
    g_oWndProc = (WndProc_t)GetWindowLongPtr(o_data_pack->hwnd, GWLP_WNDPROC);
    SetWindowLongPtr(o_data_pack->hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
    MODULEINFO module_info = get_module_info("Taskmgr.exe");

    g_base_address    = (ULONG64)module_info.lpBaseOfDll;
    g_RefreshRate_ptr = *address_offset<ULONG64>(g_base_address + 0x11C830, 0x110);

    o_UpdateData      = (UpdateData_t)    (find_pattern(&module_info, g_patten.UpdateData) + g_patten.UpdateData_offset);
    o_IsServer        = (IsServer_t)      (find_pattern(&module_info, g_patten.IsServer) + g_patten.IsServer_offset);
    SetRefreshRate    = (SetRefreshRate_t)(find_pattern(&module_info, g_patten.SetRefreshRate) + g_patten.SetRefreshRate_offset);
    GetBlockColors    = (GetBlockColors_t)(find_pattern(&module_info, g_patten.GetBlockColors) + g_patten.GetBlockColors_offset);
    SetBlockData      = (SetBlockData_t)  (find_pattern(&module_info, g_patten.SetBlockData) + g_patten.SetBlockData_offset);

    o_UpdateQuery     = (UpdateQuery_t)(g_base_address + 0x7D9AC);
    HMODULE lib       = LoadLibraryW(L"CHARTV.dll");
    CvSetData         = (CvSetData_t)GetProcAddress(lib, "CvSetData");
    std::cout << CvSetData << std::endl;

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&o_UpdateData, UpdateData);
    DetourAttach((PVOID*)&o_IsServer, IsServer);
    DetourAttach((PVOID*)&o_UpdateQuery, UpdateQuery);

    DetourTransactionCommit();
    
    SetRefreshRate(g_RefreshRate_ptr, REFRESH_RATE);
    while (!g_core) {
        Sleep(500);
    }

    UINT16* cpu_count = (UINT16*)((BYTE*)g_core + 0x944);
    Sleep(500);

    std::cout << u8"注入成功，可以使用了, core=" << *cpu_count << std::endl;
    *cpu_count = o_data_pack->frame_size;
    return true;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        SetConsoleOutputCP(CP_UTF8);

        //通訊
        HANDLE hFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, L"Global\\dllmemfilemap123");
        o_data_pack = (DataPack*)MapViewOfFile(hFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        DWORD dwThreadId;
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