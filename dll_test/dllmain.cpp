// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include "function.h"
#include "Proc.h"

//RefreshRate
SetRefreshRate_t SetRefreshRate;

//Chart
CvSetData_t CvSetData;
UpdateQuery_t o_UpdateChartData;

__int64 __fastcall UpdateChartData(void* a1, HWND a2) {
    if (g_data_pack->mode != DataPack::CHART) {
        return  o_UpdateChartData(a1, a2);
    }

    float blacks[MAX_LEN] = { 0 };
    float whites[MAX_LEN] = { 0 };

    for(int i = 0; i < g_data_pack->width; i++) {
        bool is_find_white = false;
        bool is_find_black = false;

        int white = 0;
        int black = 100;

        for (int j = 0; j < g_data_pack->height; j++) {
            int index = (j * g_data_pack->width + i) * 3; // 計算原始圖片的索引 
            uint8_t b = g_data_pack->pixel[index];
            uint8_t g = g_data_pack->pixel[index + 1];
            uint8_t r = g_data_pack->pixel[index + 2];
            uint8_t gray = bgr2gray(b, g, r);

            if (gray >= 127 && is_find_white == false) {
                white = j;
            }
            else {
                is_find_white = true;
            }

            if (gray <= 127 && is_find_black == false) {
                black = j;
            }
            else {
                is_find_black = true;
            }
        }

        whites[i] = 100 - white;
        blacks[i] = 100 - black;
    }

    VARIANTARG pvarg;
    double* v4 = (double*)*((QWORD*)a1 + 43);

    for (int i = 0; i < 60; i++) {
        unsigned __int64 v20 = *((QWORD*)a1 + 55);
        VariantInit(&pvarg);
        pvarg.vt = VT_R8; //VT_R8 -> double
        pvarg.dblVal = whites[i];
        CvSetData(v20, i, &pvarg);

        if (v4) {
            QWORD v22 = *((QWORD*)a1 + 56);
            pvarg.vt = VT_R8;
            pvarg.dblVal = blacks[i]; //set value
            CvSetData(v22, i, &pvarg);
        }
    }

    SendMessageW(a2, 0x410u, NULL, NULL); //Redraw
    g_data_pack->frame_done = TRUE; // 代表更新

    return 0;
}

//HeatMap
SetBlockData_t SetBlockData;
GetBlockColors_t GetBlockColors;
UpdateData_t o_UpdateData;

int64_t __fastcall UpdateData(void* self) {
    if (g_data_pack->mode != DataPack::BLOCK) {
        return o_UpdateData(self);
    }

    int64_t ret = o_UpdateData(self);
    UINT a4, a5;
    wchar_t text[5];

    int pixel_index = 0;
    for (int i = 0; i < g_data_pack->frame_size; i++) {
        uint8_t b     = g_data_pack->pixel[pixel_index];
        uint8_t g     = g_data_pack->pixel[pixel_index + 1];
        uint8_t r     = g_data_pack->pixel[pixel_index + 2];
        uint8_t gray  = bgr2gray(b, g, r);
        
        UINT value = 100 - (float)gray / 255.0 * 100;
        itows(value, text, 5);

        //方塊顏色
        GetBlockColors(self, value, &a4, &a5);

        a4 = abgr_to_hex(255, b, g, r);
        SetBlockData(self, i, text, a4, a5);

        pixel_index += 3;
    }

    g_data_pack->frame_done = TRUE; // 代表更新

    return ret;
}

DWORD WINAPI attach(LPVOID) {
    g_oWndProc = (WndProc_t)GetWindowLongPtr(g_data_pack->hwnd, GWLP_WNDPROC);
    SetWindowLongPtr(g_data_pack->hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
    MODULEINFO module_info = get_module_info("Taskmgr.exe");

    g_base_address      = (QWORD)           (module_info.lpBaseOfDll);
    SetRefreshRate      = (SetRefreshRate_t)(g_base_address + 0x5F978);
    g_core              = (UINT16*)         (g_base_address + 0x11C374);

    GetBlockColors      = (GetBlockColors_t)(g_base_address + 0xC9158);
    SetBlockData        = (SetBlockData_t)  (g_base_address + 0xC9B70);
    o_UpdateData        = (UpdateData_t)    (g_base_address + 0xC9CC8);
    o_UpdateChartData   = (UpdateQuery_t)   (g_base_address + 0x7D9AC);

    HMODULE cv_lib      = LoadLibraryW(L"CHARTV.dll");
    CvSetData           = (CvSetData_t)GetProcAddress(cv_lib, "CvSetData");

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach((PVOID*)&o_UpdateData, UpdateData);
    DetourAttach((PVOID*)&o_UpdateChartData, UpdateChartData);

    DetourTransactionCommit();

    *g_core = g_data_pack->frame_size;
    SetRefreshRate(GetCurrentThreadId(), g_data_pack->refresh_rate);

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
        g_data_pack = (DataPack*)MapViewOfFile(hFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

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
