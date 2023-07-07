#include "pch.h"
#include "Global.h"

WndProc_t g_oWndProc        = nullptr;
QWORD     g_base_address    = 0;
INT64     g_RefreshRate_ptr = 0;
UINT16*   g_core            = nullptr;

DataPack* o_data_pack       = nullptr;