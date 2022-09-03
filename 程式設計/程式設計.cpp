﻿#include "pch.h"
#include <filesystem>

using namespace std;

int main() {
    ShellExecuteA(nullptr, "open", "taskmgr", nullptr, nullptr, SW_SHOWNORMAL);
    Sleep(2000);
    DWORD pid;
    //#define Release
    string Path = "C:\\Users\\creep\\source\\repos\\程式設計\\x64\\Release\\dll_test.dll";
#ifndef Release
    //Path = "C:\\Users\\creep\\source\\repos\\程式設計\\x64\\Release\\Dll1.dll";
#else
    Path = (std::filesystem::current_path().string() + "\\dll_test.dll");
#endif
    std::cout << "Current path is " << Path << '\n';
    HWND hWnd = FindWindowW(L"TaskManagerWindow", L"工作管理員");
    GetWindowThreadProcessId(hWnd, &pid);
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    LPVOID loc = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(hProcess, loc, Path.c_str(), Path.length(), NULL);
    HMODULE hModule = LoadLibraryW(L"Kernel32.dll");
    LPTHREAD_START_ROUTINE lpStartAddress = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "LoadLibraryA");
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, lpStartAddress, loc, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    if (hThread) {
        printf("Release...\n");
        VirtualFreeEx(hProcess, loc, MAX_PATH, MEM_RELEASE);
        CloseHandle(hThread);
    }
#ifndef Release
    system("pause");
#endif
}
