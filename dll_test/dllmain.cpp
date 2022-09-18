// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include "function.h"
#include "Proc.h"

//RefreshRate
SetRefreshRate_t o_SetRefreshRate;
__int64 __fastcall SetRefreshRate(__int64 a1, signed int a2) {
    g_RefreshRate_ptr = a1;
    return o_SetRefreshRate(a1, a2);
}

//IsServer_t
IsServer_t       o_IsServer;
bool __fastcall IsServer(void* self) {
    g_core = self;
    std::cout << self << std::endl;
    return o_IsServer(self);
}

//HeatMap
SetBlockData_t   SetBlockData;
GetBlockColors_t GetBlockColors;
UpdateData_t     o_UpdateData;

int64_t __fastcall UpdateData(void* self) {
    int64_t ret = o_UpdateData(self);
    UINT a4, a5;
    wchar_t text[5];
    
    for (int i = 0; i < o_data_pack->frame_size; i++) {
        UINT value = 100 - (float)o_data_pack->pixel[i] / 255.0 * 100;
        swprintf_s(text, L"%d%%", value);
        GetBlockColors(self, value, &a4, &a5);
        SetBlockData(self, i, text, a4, a5);
    }

    o_data_pack->frame_done = TRUE; // 代表更新

    return ret;
}

//Chart
CvSetData_t CvSetData;
UpdateQuery_t o_UpdateChartData;

__int64 __fastcall UpdateChartData(void* a1, HWND a2) {
    std::mt19937 rng(time(0));
    std::uniform_real_distribution<float> unif(0.0, 100.0);
    QWORD CVArray = *((QWORD*)a1 + 55);
    VARIANTARG varg;

    DWORD* v6 = (DWORD*)*((QWORD*)a1 + 40);
    if (*(QWORD*)v6 != g_base_address + 0xCECC8) { //WdcCpuMonitor
        return o_UpdateChartData(a1, a2);
    }

    for (int i = 0; i < 60; i++) {
        VariantInit(&varg);
        varg.vt = VT_R8; //VT_R8 -> double
        varg.dblVal = unif(rng); //set value
        CvSetData(CVArray, 59 - i, &varg); //draw
    }

    SendMessageW(a2, 0x410u, NULL, NULL); //Redraw
    return 0;
}

DWORD WINAPI attach(LPVOID) {
    g_oWndProc = (WndProc_t)GetWindowLongPtr(o_data_pack->hwnd, GWLP_WNDPROC);
    SetWindowLongPtr(o_data_pack->hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
    MODULEINFO module_info = get_module_info("Taskmgr.exe");
    g_base_address = (QWORD)module_info.lpBaseOfDll;

    if((QWORD)address_offset<QWORD>(g_base_address + 0x11C830, 0x110) >> 9) { //True
        g_RefreshRate_ptr = *address_offset<QWORD>(g_base_address + 0x11C830, 0x110);
    }
    else {
        std::cout << u8"不正確的偏移😾\n請點擊 檢視->更新速度->高" << std::endl;
    }

    o_UpdateData      = (UpdateData_t)    (find_pattern(&module_info, g_patten.UpdateData) + g_patten.UpdateData_offset);
    o_IsServer        = (IsServer_t)      (find_pattern(&module_info, g_patten.IsServer) + g_patten.IsServer_offset);
    o_SetRefreshRate  = (SetRefreshRate_t)(find_pattern(&module_info, g_patten.SetRefreshRate) + g_patten.SetRefreshRate_offset);
    GetBlockColors    = (GetBlockColors_t)(find_pattern(&module_info, g_patten.GetBlockColors) + g_patten.GetBlockColors_offset);
    SetBlockData      = (SetBlockData_t)  (find_pattern(&module_info, g_patten.SetBlockData) + g_patten.SetBlockData_offset);

    printf("%p %p %p %p %p\n", o_UpdateData, o_IsServer, o_SetRefreshRate, GetBlockColors, SetBlockData);

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&o_SetRefreshRate, SetRefreshRate);
    DetourTransactionCommit();

    /*o_UpdateChartData = (UpdateQuery_t)(g_base_address + 0x7D9AC);
    HMODULE cv_lib    = LoadLibraryW(L"CHARTV.dll");
    CvSetData         = (CvSetData_t)GetProcAddress(cv_lib, "CvSetData");*/

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&o_UpdateData, UpdateData);
    DetourAttach((PVOID*)&o_IsServer, IsServer);
    DetourAttach((PVOID*)&o_UpdateChartData, UpdateChartData);
    DetourTransactionCommit();

    std::cout << u8"請等待回應" << std::endl;
    while (!g_core) {
        Sleep(500);
    }

    std::cout << u8"好" << std::endl;
    while(!g_RefreshRate_ptr){
        Sleep(500);
    }

    std::cout << u8"OK" << std::endl;
    SetRefreshRate(g_RefreshRate_ptr, REFRESH_RATE);

    UINT16* cpu_count = (UINT16*)((BYTE*)g_core + 0x944);
    Sleep(500);

    std::cout << u8"注入成功，可以使用了, core=" << *cpu_count << std::endl;
    *cpu_count = o_data_pack->frame_size;
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

/*
    o_UpdateData      =      (UpdateData_t)(g_base_address + 0xC9CC8);
    o_IsServer        =        (IsServer_t)(g_base_address + 0x195BC);
    SetRefreshRate    =  (SetRefreshRate_t)(g_base_address + 0x6063C);
    GetBlockColors    =  (GetBlockColors_t)(g_base_address + 0xC9158);
    SetBlockData      =    (SetBlockData_t)(g_base_address + 0xC9B70);
 */