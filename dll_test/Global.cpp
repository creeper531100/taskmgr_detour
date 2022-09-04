#include "pch.h"
#include "Global.h"

HINSTANCE g_hInstance       = nullptr;
HWND      g_HWND            = nullptr;
WndProc_t g_oWndProc        = nullptr;
ULONG64   g_base_address    = 0;
void*     g_core            = 0;
INT64     g_RefreshRate_ptr = 0;

UCHAR*    g_img_array       = nullptr;

Patten g_patten = {
    "\x48\x8B\xCE\xE8\x4D\xF6\xFF\xFF\x8B\xD8", -0x4B,
    "\x48\x33\xCC\xE8\x33\x99\x01\x00", -0x39,
    "\xFF\x15\xE4\x61\x07\x00\x48\x8B\xC8\x48\x8B\xD3", -0x32,
    "\x48\x8B\x0C\xF0\x48\xFF\x15\x08\xB4\x00\x00", -0x4D,
    "\xC7\x05\x9B\x0C\x05\x00\x00\x00\x00\x80\xE8", -0x10F
};
