// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include "function.h"
#include "Proc.h"

#define LODWORD(x) (*((unsigned int*)&(x)))

#define LLPRINT(v) printf("%s=%p\n" , #v, v);
#define PRINT(fmt, v) printf("%s="##fmt##"\n" , #v, v);

#define WIDTH 60
#define HEIGHT 100

//RefreshRate
SetRefreshRate_t SetRefreshRate;

//Chart
CvSetData_t CvSetData;
UpdateQuery_t o_UpdateChartData;

#define _DWORD DWORD
#define _QWORD QWORD

__int64 __fastcall UpdateChartData(void* a1, HWND a2) {
    DWORD* v6 = (DWORD*)*((QWORD*)a1 + 40);
    double* v4 = (double*)*((QWORD*)a1 + 43);
    QWORD v22;

    unsigned __int64 v20;
    VARIANTARG pvarg;

    float blacks[WIDTH] = { 0 };
    float whites[WIDTH] = { 0 };
    for(int i = 0; i < WIDTH; i++) {
        bool is_find_white = false;
        bool is_find_black = false;

        int white = 0;
        int black = 100;

        for (int j = 0; j < 100; j++) {
            int index = j * WIDTH + i; // 計算原始圖片的索引
            if ((float)o_data_pack->pixel[index] >= 127.0 && is_find_white == false) {
                white = j;
            }
            else {
                is_find_white = true;
            }

            if ((float)o_data_pack->pixel[index] <= 127.0 && is_find_black == false) {
                black = j;
            }
            else {
                is_find_black = true;
            }
        }

        whites[i] = 100 - white;
        blacks[i] = 100 - black;
    }

    for (int i = 0; i < 60; i++) {
        v20 = *((QWORD*)a1 + 55);
        VariantInit(&pvarg);
        pvarg.vt = VT_R8; //VT_R8 -> double
        pvarg.dblVal = whites[i];
        CvSetData(v20, i, &pvarg);

        if (v4) {
            v22 = *((QWORD*)a1 + 56);
            pvarg.vt = 5;
            pvarg.dblVal = blacks[i]; //set value
            CvSetData(v22, i, &pvarg);
        }
    }

    SendMessageW(a2, 0x410u, NULL, NULL); //Redraw
    o_data_pack->frame_done = TRUE; // 代表更新
    return 0;
}

//HeatMap
SetBlockData_t SetBlockData;
GetBlockColors_t GetBlockColors;
UpdateData_t o_UpdateData;

int64_t __fastcall UpdateData(void* self) {
    int64_t ret = o_UpdateData(self);
    UINT a4, a5;
    wchar_t text[5];

    for (int i = 0; i < o_data_pack->frame_size; i++) {
        UINT value = 100 - (float)o_data_pack->pixel[i] / 255.0 * 100;
        itows(value, text, 5);

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

    g_base_address = (QWORD)(module_info.lpBaseOfDll);
    SetRefreshRate = (SetRefreshRate_t)(g_base_address + 0x5F978);
    g_core = (UINT16*)(g_base_address + 0x11C374);

    GetBlockColors = (GetBlockColors_t)(g_base_address + 0xC9158);
    SetBlockData = (SetBlockData_t)(g_base_address + 0xC9B70);
    o_UpdateData = (UpdateData_t)(g_base_address + 0xC9CC8);
    o_UpdateChartData = (UpdateQuery_t)(g_base_address + 0x7D9AC);

    HMODULE cv_lib = LoadLibraryW(L"CHARTV.dll");
    CvSetData = (CvSetData_t)GetProcAddress(cv_lib, "CvSetData");

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach((PVOID*)&o_UpdateData, UpdateData);
    DetourAttach((PVOID*)&o_UpdateChartData, UpdateChartData);

    DetourTransactionCommit();

    *g_core = o_data_pack->frame_size;
    SetRefreshRate(GetCurrentThreadId(), 1);

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
        break;
    }
    }
    return TRUE;
}
