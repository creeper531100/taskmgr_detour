// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"

#include <iostream>
#include <sstream>
#include "Proc.h"
#include <detours/detours.h>

typedef int64_t (__fastcall* UpdateData_t)(void*, unsigned int);
typedef void (__fastcall *GetBlockColors_t)(void*, int, long*, long*);
typedef __int64 (__fastcall *SetBlockData_t)(void*, UINT, wchar_t*, UINT, UINT);
typedef bool (__fastcall *IsServer_t)(void*);
typedef __int64 (__fastcall* GetCpuUsageData_t)(void*, unsigned int, int*, int*);

UpdateData_t oUpdateData;
SetBlockData_t oSetBlockData;
GetBlockColors_t oGetBlockColors;
IsServer_t oIsServer;
GetCpuUsageData_t g_GetCpuUsageData;
void* g_Core;

bool __fastcall IsServer(void* ret) {
    g_Core = ret;
    return oIsServer(ret);
}

void __fastcall GetBlockColors(void* ret, int a2, long* a3, long* a4) {
    std::cout << ret << std::endl;
    oGetBlockColors(ret, a2, a3, a4);
}

__int64 __fastcall SetBlockData(void* ret, UINT a2, wchar_t* a3, UINT a4, UINT a5) {
    std::cout << ret << std::endl;
    return oSetBlockData(ret, a2, a3, a4, a5);
}

int64_t __fastcall UpdateData(void* ret, unsigned int a2) {
    UINT16* cpu_count = (UINT16*)((BYTE*)g_Core + 0x944);
    std::cout << cpu_count << std::endl;
    return oUpdateData(ret, a2);
}

DWORD WINAPI attach(LPVOID dllHandle) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    g_oWndProc = (WNDPROC_T)GetWindowLongPtr(g_HWND, GWLP_WNDPROC);
    SetWindowLongPtr(g_HWND, GWLP_WNDPROC, (LONG_PTR)WndProc);
    EnumWindows(EnumWindowsProc, GetCurrentProcessId());

    g_BaseAddress       = (LONG_PTR)         GetModuleInfo("taskmgr.exe").lpBaseOfDll;
    oUpdateData         = (UpdateData_t)     (g_BaseAddress + 0xC9CC8); // 0xEB10 taskmgr.exe + C9CC8 
    oSetBlockData       = (SetBlockData_t)   (g_BaseAddress + 0xC9B70);
    oGetBlockColors     = (GetBlockColors_t) (g_BaseAddress + 0xC9158);
    oIsServer           = (IsServer_t)       (g_BaseAddress + 0x195BC);
    g_GetCpuUsageData   = (GetCpuUsageData_t)(g_BaseAddress + 0xC9368);
    
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&oUpdateData, UpdateData);
    DetourAttach((PVOID*)&oGetBlockColors, GetBlockColors);
    DetourAttach((PVOID*)&oSetBlockData, SetBlockData);
    DetourAttach((PVOID*)&oIsServer, IsServer);
    DetourTransactionCommit();

    std::cout << oUpdateData << std::endl;
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
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
