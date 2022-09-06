#pragma once
#include "pch.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    return g_oWndProc(hwnd, message, wParam, lParam);
}
