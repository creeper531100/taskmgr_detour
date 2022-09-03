// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"

#include <iostream>
#include <sstream>
#include "Proc.h"
#include <detours/detours.h>

typedef int64_t (__fastcall* UpdateData_t)(void*);
typedef bool (__fastcall *IsServer_t)(void*);

UpdateData_t oUpdateData;
IsServer_t oIsServer;
void* g_Core;

bool __fastcall IsServer(void* ret) {
    g_Core = ret;
    return oIsServer(ret);
}

int64_t __fastcall UpdateData(void* ret) {
    std::cout << "True" << std::endl;
    return oUpdateData(ret);
}

DWORD WINAPI attach(LPVOID dllHandle) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    g_oWndProc = (WNDPROC_T)GetWindowLongPtr(g_HWND, GWLP_WNDPROC);
    SetWindowLongPtr(g_HWND, GWLP_WNDPROC, (LONG_PTR)WndProc);
    EnumWindows(EnumWindowsProc, GetCurrentProcessId());
    g_BaseAddress = (ULONG64)GetModuleInfo("taskmgr.exe").lpBaseOfDll;
    oUpdateData = (UpdateData_t)(g_BaseAddress + 0xC9CC8);
    oIsServer = (IsServer_t)(g_BaseAddress + 0x195BC);
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&oUpdateData, UpdateData);
    DetourAttach((PVOID*)&oIsServer, IsServer);
    DetourTransactionCommit();

    while (!g_Core) {
        Sleep(500);
    }

    UINT16* cpu_count = (UINT16*)((BYTE*)g_Core + 0x944);
    Sleep(500);

    *cpu_count = 512;
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
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// 0xEB10 taskmgr.exe + C9CC8 0E650
