#include "pch.h"
#include "Global.h"

WndProc_t g_oWndProc        = nullptr;
QWORD     g_base_address    = 0;
void*     g_core            = 0;
INT64     g_RefreshRate_ptr = 0;

DataPack* o_data_pack       = nullptr;

Patten g_patten = {
    "48 8B CE E8 4D F6 FF FF 8B D8", -0x4B,
    "48 33 CC E8 33 99 01 00", -0x39,
    "FF 15 E4 61 07 00 48 8B C8 48 8B D3", -0x32,
    "48 8B 0C F0 48 FF 15 08 B4 00 00", -0x4D,
    "C7 05 9B 0C 05 00 00 00 00 80 E8", -0x10F
};
