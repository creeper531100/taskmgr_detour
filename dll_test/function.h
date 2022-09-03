#include "pch.h"
#include <Psapi.h>

MODULEINFO GetModuleInfo(const char* szModule) {
    MODULEINFO modinfo = { 0 };
    HMODULE hModule = GetModuleHandleA(szModule);
    if (hModule == 0) return modinfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
    return modinfo;
}

BOOLEAN bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask) {
    for (; *szMask; ++szMask, ++pData, ++bMask)
        if (*szMask == 'x' && *pData != *bMask)
            return 0;
    return (*szMask) == 0;
}

UINT64 FindPattern(UINT64 dwAddress, UINT64 dwLen, BYTE* bMask, const char* szMask) {
    for (UINT64 i = 0; i < dwLen; i++)
        if (bDataCompare((BYTE*)(dwAddress + i), bMask, szMask))
            return (UINT64)(dwAddress + i);
    return 0;
}

void shellcode_write(PVOID ptr, PVOID byte, SIZE_T size) {
    DWORD cur, tmp;
    VirtualProtect((PVOID)ptr, size, PAGE_EXECUTE_READWRITE, &cur);
    memset((PVOID)ptr, 0x90, size);
    memcpy((PVOID)ptr, byte, size);
    VirtualProtect((PVOID)ptr, size, cur, &tmp);
}