#include "pch.h"
#include <filesystem>

using namespace std;
//#define Release

int main() {
    SetConsoleOutputCP(CP_UTF8);
    ShellExecuteA(nullptr, "open", "taskmgr", nullptr, nullptr, SW_SHOWNORMAL);
    printf(u8"請等待注入...不要點擊任何按鈕\n");
    Sleep(2000);
    DWORD pid;
    string Path = "C:\\Users\\creep\\source\\repos\\程式設計\\x64\\Release\\dll_test.dll";
#ifdef Release
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
        printf("成功...\n");
        VirtualFreeEx(hProcess, loc, MAX_PATH, MEM_RELEASE);
        CloseHandle(hThread);
    }
#ifdef Release
    system("pause");
#endif
}
