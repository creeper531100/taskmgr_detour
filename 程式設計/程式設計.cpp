#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <TlHelp32.h>
#include <opencv2/opencv.hpp>
using namespace std;

//#define Release

constexpr int FRAME_SIZE = 38 * 39;
constexpr int MAX_SIZE = 100 * 100;
HWND g_HWND;

cv::Mat screenshot(HWND);
HANDLE GetProcessByName(wstring, DWORD*);
static BOOL CALLBACK enumWindowCallback(HWND, LPARAM);

struct DataPack {
    UINT8 pixel[MAX_SIZE];
    UINT16 frame_size;
    HWND hwnd;
    BOOL frame_done;
} * lpvMem;

int main() {
    string Path = "C:\\Users\\creep\\source\\repos\\taskmgr_detour\\x64\\Release\\dll_test.dll";
    string video_path = "E:\\360p.mp4";
    SetConsoleOutputCP(CP_UTF8);
    
#ifdef Release
    Path = (std::filesystem::current_path().string() + "\\dll_test.dll");
    cout << u8"\"視頻\"=";
    cin >> video_path;
#endif

    printf(u8"\n請等待注入...不要點擊任何按鈕\n");
    ShellExecuteA(nullptr, "open", "taskmgr", nullptr, nullptr, SW_SHOWNORMAL);

    static HANDLE hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DataPack),
                                                 L"Global\\dllmemfilemap123");


    lpvMem = (DataPack*)MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    lpvMem->frame_done = FALSE;
    lpvMem->frame_size = FRAME_SIZE;

    Sleep(2000);

    DWORD pid;
    HANDLE hProcess = GetProcessByName(L"Taskmgr.exe", &pid);
    EnumWindows(enumWindowCallback, pid);
    lpvMem->hwnd = g_HWND;

    std::cout << "Current path is " << Path << '\n';

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

    cv::VideoCapture cap(video_path);
    cv::Mat img, tmp;

    cv::Size dsize = screenshot(g_HWND).size(); //screenshot(hwnd)
    double fps = cap.get(cv::CAP_PROP_FPS);
    cv::VideoWriter video("out.avi", cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), fps, dsize, true);

    int useless_var = 0;
    while (1) {
        if (lpvMem->frame_done) {
            cap >> img;
            if (img.empty())
                break;
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);

            cv::resize(img, img, {39, 38});
            memcpy(lpvMem->pixel, img.data, lpvMem->frame_size);
            lpvMem->frame_done = FALSE;

            cv::cvtColor(screenshot(g_HWND), tmp, cv::COLOR_BGRA2BGR);
            cv::resize(tmp, tmp, dsize);
            video.write(tmp);
        }
        useless_var = 1; //卡住編譯器優化
    }
    printf("%d\n", useless_var);
}

HANDLE GetProcessByName(wstring name, DWORD* pid) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);
    if (Process32First(snapshot, &process)) {
        do {
            if (wstring(process.szExeFile) == name) {
                *pid = process.th32ProcessID;
                break;
            }
        }
        while (Process32Next(snapshot, &process));
    }
    CloseHandle(snapshot);

    if (*pid != 0) {
        return OpenProcess(PROCESS_ALL_ACCESS, FALSE, *pid);
    }
    return NULL;
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    if (lparam == dwProcessId && IsWindowVisible(hWnd)) {
        g_HWND = hWnd;
        return FALSE;
    }
    return TRUE;
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
