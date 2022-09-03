#pragma once
#include "pch.h"


typedef LRESULT(CALLBACK* WNDPROC_T)(HWND, UINT, WPARAM, LPARAM);

extern HINSTANCE g_hInstance;
extern HWND      g_HWND;
extern WNDPROC_T g_oWndProc;
extern ULONG64   g_BaseAddress;

