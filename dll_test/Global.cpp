#include "pch.h"
#include "Global.h"

HINSTANCE g_hInstance       = nullptr;
HWND      g_HWND            = nullptr;
WndProc_t g_oWndProc        = nullptr;
ULONG64   g_BaseAddress     = 0;
void*     g_Core            = 0;
INT64     g_RefreshRate_ptr = 0;