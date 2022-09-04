#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <TlHelp32.h>
#include <opencv2/opencv.hpp>

static LPVOID lpvMem = NULL;
static HANDLE hMapObject = NULL;
constexpr int FRAME_SIZE = 38 * 39;

//#define Release
using namespace std;
HANDLE GetProcessByName(wstring);

int main() {
    SetConsoleOutputCP(CP_UTF8);
    ShellExecuteA(nullptr, "open", "taskmgr", nullptr, nullptr, SW_SHOWNORMAL);
    printf(u8"請等待注入...不要點擊任何按鈕\n");
    string Path = "C:\\Users\\creep\\source\\repos\\程式設計\\x64\\Release\\dll_test.dll";

    cv::Mat img(39, 38, CV_8U);
    img = cv::Scalar(255, 255, 255);
    cv::VideoCapture cap("D:\\7.mp4");

    uchar* data = img.data;
    SIZE_T size = img.size().area() * sizeof(uchar) + 1;

    hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, L"dllmemfilemap123");
    lpvMem = MapViewOfFile(hMapObject, FILE_MAP_WRITE, 0, 0, 0);
    memcpy(lpvMem, data, size);

    Sleep(2000);
#ifdef Release
    Path = (std::filesystem::current_path().string() + "\\dll_test.dll");
#endif
    std::cout << "Current path is " << Path << '\n';
    HANDLE hProcess = GetProcessByName(L"Taskmgr.exe");
    LPVOID loc = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(hProcess, loc, Path.c_str(), Path.length(), NULL);
    HMODULE hModule = LoadLibraryW(L"Kernel32.dll");
    LPTHREAD_START_ROUTINE lpStartAddress = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "LoadLibraryA");
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, lpStartAddress, loc, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    if (hThread) {
        printf(u8"成功...\n");
        VirtualFreeEx(hProcess, loc, MAX_PATH, MEM_RELEASE);
        CloseHandle(hThread);
    }

    ((UCHAR*)lpvMem)[FRAME_SIZE] = 0; // 代表未更新

    while(1) {
        if(((UCHAR*)lpvMem)[FRAME_SIZE]) {
            cap >> img;
            cv::imshow("main", img);
            cv::waitKey(0);

            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
            cv::resize(img, img, { 39, 38 });

            ((UCHAR*)lpvMem)[FRAME_SIZE] = 0;
        }
        cout << (int)((UCHAR*)lpvMem)[FRAME_SIZE] << endl;
    }
}

HANDLE GetProcessByName(wstring name) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);
    if (Process32First(snapshot, &process)) {
        do {
            if (wstring(process.szExeFile) == name) {
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }
    CloseHandle(snapshot);
    if (pid != 0) {
        return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    }
    return NULL;
}
