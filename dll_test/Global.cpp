#include "pch.h"
#include "Global.h"

HINSTANCE g_hInstance   = nullptr;
HWND      g_HWND        = nullptr;
WNDPROC_T g_oWndProc    = nullptr;
LONG_PTR  g_BaseAddress = 0;
ULONG64*  g_pVTableBase = nullptr;