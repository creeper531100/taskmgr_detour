#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <TlHelp32.h>
#include <opencv2/opencv.hpp>


//#define Release

static HANDLE hMapObject = NULL;
constexpr int FRAME_SIZE = 38 * 39;
cv::Mat screenshot(HWND path);

struct DataPack {
    UINT8 pixel[FRAME_SIZE];
    BOOL frame_done;
} * lpvMem;

using namespace std;
HANDLE GetProcessByName(wstring);

string Path = "C:\\Users\\creep\\source\\repos\\taskmgr_detour\\x64\\Release\\dll_test.dll";
#ifdef Release
       Path = (std::filesystem::current_path().string() + "\\dll_test.dll");
#endif

int main() {
    SetConsoleOutputCP(CP_UTF8);
    printf(u8"請等待注入...不要點擊任何按鈕\n");
    ShellExecuteA(nullptr, "open", "taskmgr", nullptr, nullptr, SW_SHOWNORMAL);

    hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DataPack),
                                   L"Global\\dllmemfilemap123");
    lpvMem = (DataPack*)MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    lpvMem->frame_done = FALSE;

    Sleep(2000);
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

    cv::VideoCapture cap("E:\\7.mp4");
    cv::Mat img;
    HWND hwnd = FindWindowA(NULL, "工作管理員");

    cv::Size size = screenshot(hwnd).size();//screenshot(hwnd)
    cv::VideoWriter video("out.avi", cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 30.0, size, true);
    cv::Mat mat2;

    while (1) {
        if (lpvMem->frame_done) {
            cap >> img;
            if (img.empty())
                return 0;
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
            
            cv::resize(img, img, {39, 38});
            memcpy(lpvMem->pixel, img.data, FRAME_SIZE);
            lpvMem->frame_done = FALSE;

            cv::cvtColor(screenshot(hwnd), mat2, cv::COLOR_BGRA2BGR);
            cv::resize(mat2, mat2, size);
            video.write(mat2);
           
        }
        printf("");
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
        }
        while (Process32Next(snapshot, &process));
    }
    CloseHandle(snapshot);
    if (pid != 0) {
        return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    }
    return NULL;
}

cv::Mat screenshot(HWND hwnd) {
    HDC hwindowDC, hwindowCompatibleDC;

    int height, width, srcheight, srcwidth;
    HBITMAP hbwindow;
    cv::Mat src;
    BITMAPINFOHEADER bi;

    hwindowDC = GetDC(hwnd);
    hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    RECT windowsize; // get the height and width of the screen
    GetClientRect(hwnd, &windowsize);

    srcheight = windowsize.bottom;
    srcwidth = windowsize.right;
    height = windowsize.bottom / 1; //change this to whatever size you want to resize to
    width = windowsize.right / 1;

    src.create(height, width, CV_8UC4);

    // create a bitmap
    hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    bi.biSize = sizeof(BITMAPINFOHEADER);
    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
    bi.biWidth = width;
    bi.biHeight = -height; //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);
    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY);
    //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    //copy from hwindowCompatibleDC to hbwindow

    // avoid memory leak
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return src;
}
