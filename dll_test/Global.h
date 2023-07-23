#pragma once
#include <Oaidl.h>

#define LODWORD(x) (*((unsigned int*)&(x)))

#define _DWORD DWORD
#define _QWORD QWORD

using QWORD            = ULONG64;
using WndProc_t        = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
using UpdateData_t     = INT64(__fastcall*)(void*);
using SetRefreshRate_t = INT64(__fastcall*)(INT64, UINT);
using GetBlockColors_t = void(__fastcall*)(void*, int, UINT*, UINT*);
using SetBlockData_t   = __int64(__fastcall*)(void*, UINT, wchar_t*, UINT, UINT);
using UpdateQuery_t    = __int64(__fastcall*)(void*, HWND a2);
using CvSetData_t      = long(*)(unsigned __int64, unsigned long, tagVARIANT*);

constexpr int MAX_SIZE = 100 * 100 * 3;
constexpr int MAX_LEN  = 100;

extern WndProc_t g_oWndProc;
extern QWORD     g_base_address;
extern UINT16*   g_core;
extern INT64     g_RefreshRate_ptr;

struct DataPack {
    UINT8 pixel[MAX_SIZE];

    UINT16 frame_size;
    UINT16 width;
    UINT16 height;

    HWND hwnd;
    BOOL frame_done;

    enum Mode {
        BLOCK,
        CHART
    };
    Mode mode;

    UINT16 refresh_rate;
};

extern DataPack* g_data_pack;