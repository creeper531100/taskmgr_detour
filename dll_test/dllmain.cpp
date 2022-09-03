// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include "function.h"

#include <iostream>
#include <sstream>
#include "Proc.h"
#include <detours/detours.h>

#define FRAME_SIZE 38 * 39

using UpdateData_t      =   int64_t(__fastcall*)(void*);
using IsServer_t        =   bool(__fastcall*)(void*);
using SetRefreshRate_t  =   int64_t(__fastcall*)(int64_t, UINT);
using GetBlockColors_t  =   void(__fastcall*)(void*, int, UINT*, UINT*);
using SetBlockData_t    =   __int64(__fastcall*)(void*, UINT, wchar_t*, UINT, UINT);

UpdateData_t o_UpdateData;
IsServer_t o_IsServer;
SetRefreshRate_t o_SetRefreshRate;

SetBlockData_t SetBlockData;
GetBlockColors_t GetBlockColors;

void* g_Core;
int64_t g_RefreshRate_ptr;

bool __fastcall IsServer(void* ret) {
    g_Core = ret;
    return o_IsServer(ret);
}

int64_t __fastcall UpdateData(void* ret) {
    int64_t return_data = o_UpdateData(ret);
    UINT a4, a5;
    int deep = 50;
    wchar_t w[5];
    for (int i = 0; i < FRAME_SIZE; i++) {
        swprintf_s(w, L"%d%%", deep);
        GetBlockColors(ret, deep, &a4, &a5);
        SetBlockData(ret, i, w, a4, a5);
    }
    return return_data;
}

int64_t __fastcall SetRefreshRate(int64_t ret, UINT a2) {
    g_RefreshRate_ptr = ret;
    return o_SetRefreshRate(ret, 60);
}

DWORD WINAPI attach(LPVOID) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    g_oWndProc = (WNDPROC_T)GetWindowLongPtr(g_HWND, GWLP_WNDPROC);
    SetWindowLongPtr(g_HWND, GWLP_WNDPROC, (LONG_PTR)WndProc);
    EnumWindows(EnumWindowsProc, GetCurrentProcessId());

    g_BaseAddress    = (ULONG64)GetModuleInfo("taskmgr.exe").lpBaseOfDll;
    o_UpdateData     = (UpdateData_t)(g_BaseAddress + 0xC9CC8);
    o_IsServer       = (IsServer_t)(g_BaseAddress + 0x195BC);
    o_SetRefreshRate = (SetRefreshRate_t)(g_BaseAddress + 0x6063C);
    GetBlockColors   = (GetBlockColors_t)(g_BaseAddress + 0xC9158);
    SetBlockData     = (SetBlockData_t)(g_BaseAddress + 0xC9B70);

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&o_UpdateData, UpdateData);
    DetourAttach((PVOID*)&o_IsServer, IsServer);
    DetourAttach((PVOID*)&o_SetRefreshRate, SetRefreshRate);
    DetourTransactionCommit();

    while (!g_Core) {
        Sleep(500);
    }

    UINT16* cpu_count = (UINT16*)((BYTE*)g_Core + 0x944);
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
        o_SetRefreshRate(g_RefreshRate_ptr, 500);
        break;
    }
    }
    return TRUE;
}
