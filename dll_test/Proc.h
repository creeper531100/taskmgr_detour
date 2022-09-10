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
#define LODWORD(x) (*((unsigned int*)&(x)))

__int64 __fastcall UpdateQuery(void* a1, HWND a2) {
    QWORD CVArray;
    VARIANTARG pvarg;
    std::mt19937 rng(time(0));
    std::uniform_real_distribution<float> unif(0.0, 100.0);

    DWORD* v6 = (DWORD*)*((QWORD*)a1 + 40);
    if (*(QWORD*)v6 == 0x7ff65eceecc8) { //WdcCpuMonitor
        for (int i = 0; i < 60; ++i) {
            CVArray = *((QWORD*)a1 + 55);
            VariantInit(&pvarg);
            pvarg.vt = 5;
            pvarg.dblVal = unif(rng);
            std::cout << pvarg.dblVal << std::endl;
            CvSetData(CVArray, i, &pvarg);
        }
    }
    else {
        auto a = o_UpdateQuery(a1, a2);
        std::cout << a << std::endl;
        return a;
    }
    return 0;
}
