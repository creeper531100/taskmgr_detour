#pragma once
#include <Oaidl.h>

using QWORD = ULONG64;
using WndProc_t        = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
using UpdateData_t     = INT64(__fastcall*)(void*);
using IsServer_t       = bool(__fastcall*)(void*);
using SetRefreshRate_t = INT64(__fastcall*)(INT64, UINT);
using GetBlockColors_t = void(__fastcall*)(void*, int, UINT*, UINT*);
using SetBlockData_t   = __int64(__fastcall*)(void*, UINT, wchar_t*, UINT, UINT);
using UpdateQuery_t    = __int64(__fastcall*)(void*, HWND a2);
using CvSetData_t      = long(*)(unsigned __int64, unsigned long, tagVARIANT*);

constexpr int MAX_SIZE = 100 * 100;
constexpr int REFRESH_RATE = 1; //500

extern WndProc_t g_oWndProc;
extern QWORD     g_base_address;
extern void*     g_core;
extern INT64     g_RefreshRate_ptr;

struct Patten {
    std::string   UpdateData;
    INT32         UpdateData_offset;
    std::string   IsServer;
    INT32         IsServer_offset;
    std::string   SetRefreshRate;
    INT32         SetRefreshRate_offset;
    std::string   SetBlockData;
    INT32         SetBlockData_offset;
    std::string   GetBlockColors;
    INT32         GetBlockColors_offset;
};

struct DataPack {
    UINT8 pixel[MAX_SIZE];
    UINT16 frame_size;
    HWND hwnd;
    BOOL frame_done;
};

extern DataPack* o_data_pack;
extern Patten g_patten;