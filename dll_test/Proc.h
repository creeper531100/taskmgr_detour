#pragma once
#include "pch.h"
#include <Oaidl.h>
#include <random>

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    return g_oWndProc(hwnd, message, wParam, lParam);
}

using UpdateQuery_t = __int64(__fastcall*)(void*, HWND a2);
using CvSetData_t = long(*)(unsigned __int64, unsigned long, tagVARIANT*);
CvSetData_t CvSetData;
UpdateQuery_t o_UpdateQuery;

using QWORD = ULONG64;
int x = 0;

__int64 __fastcall UpdateQuery(void* a1, HWND a2) {
    VARIANTARG pvarg;
    QWORD CVArray;
    VariantInit(&pvarg);
    pvarg.vt = 5;

    std::mt19937 rng(time(0));
    std::uniform_real_distribution<float> unif(0.0, 100.0);

    DWORD* v6 = (DWORD*)*((QWORD*)a1 + 40);
    if (*(QWORD*)v6 == 0x7ff65eceecc8) { //WdcCpuMonitor
        for (int i = 0; i < 60; ++i) {
            CVArray = *((QWORD*)a1 + 55);
            pvarg.dblVal = unif(rng);
            CvSetData(CVArray, 59 - i, &pvarg);
            RedrawWindow(a2, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
        }
        x++;
    }
    else {
        return o_UpdateQuery(a1, a2);
    }

    return 0;
}
