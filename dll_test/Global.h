#pragma once
#include "pch.h"


typedef LRESULT(CALLBACK* WNDPROC_T)(HWND, UINT, WPARAM, LPARAM);
typedef __int64(__fastcall* Query)(void*);

extern HINSTANCE g_hInstance;
extern HWND      g_HWND;
extern WNDPROC_T g_oWndProc;
extern ULONG64   g_BaseAddress;
extern ULONG64*  g_pVTableBase;

