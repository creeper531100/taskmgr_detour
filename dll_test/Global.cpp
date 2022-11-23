#include "pch.h"
#include "Global.h"

WndProc_t g_oWndProc        = nullptr;
QWORD     g_base_address    = 0;
void*     g_core            = 0;
INT64     g_RefreshRate_ptr = 0;

DataPack* o_data_pack       = nullptr;
Patten g_patten = {
    "CC 48 89 5C 24 10 48 89 74 24 18 55 57 41 56 48 8D 6C 24 ?? 48 81 EC 40 01 00 00 48 8B 05 ?? ?? ?? 00", 1,
    "CC 40 53 48 81 EC 60 01 00 00 48 8B 05 ?? ?? ?? 00 48 33 C4 48 89 84 24 ?? ?? 00 00", 1,
    "CC 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 48 63 FA 48 8B E9", 1,
    "4C 8B 50 08 8A CA B8 01 00 00 00 49 8B D0 D3 E0 47 0F B6 34 13 44 23 F0", -0x31,
    "CC 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 65 48 8B 04 25 ?? 00 00 00 8B DA BA 04 00 00 00", 1
};
