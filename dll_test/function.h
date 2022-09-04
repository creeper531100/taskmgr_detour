#include "pch.h"
#include <Psapi.h>

MODULEINFO get_module_info(const char* szModule) {
    MODULEINFO modinfo = { 0 };
    HMODULE hModule = GetModuleHandleA(szModule);
    if (hModule == 0) return modinfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
    return modinfo;
}

//TODO: Àu¤Æ
BOOLEAN b_data_compare(const BYTE* pData, const BYTE* bMask, const char* szMask) {
    for (; *szMask; ++szMask, ++pData, ++bMask)
        if (*szMask == 'x' && *pData != *bMask)
            return 0;
    return (*szMask) == 0;
}

UINT64 find_pattern(MODULEINFO* info, std::string bMask, std::string szMask = "") {
    if(szMask.empty()) {
        szMask = std::string(bMask.length(), 'x');
    }
    for (UINT64 i = 0; i < info->SizeOfImage; i++)
        if (b_data_compare((BYTE*)((UINT64)info->lpBaseOfDll + i), (BYTE*)bMask.c_str(), szMask.c_str()))
            return (UINT64)info->lpBaseOfDll + i;
    return 0;
}

void shellcode_write(PVOID ptr, PVOID byte, SIZE_T size) {
    DWORD cur, tmp;
    VirtualProtect((PVOID)ptr, size, PAGE_EXECUTE_READWRITE, &cur);
    memset((PVOID)ptr, 0x90, size);
    memcpy((PVOID)ptr, byte, size);
    VirtualProtect((PVOID)ptr, size, cur, &tmp);
}


//(UINT16*)((BYTE*)g_core + 0x944);
template <typename Retn>
Retn* address_offset(ULONG64 base, ULONG64 offset) {
    return (Retn*)(*(ULONG64*)base + offset);
}

template <typename Retn, typename ... Ts>
Retn* address_offset(ULONG64 base, ULONG64 offset, Ts ... ts) {
    return address_offset(*(ULONG64*)base + offset, ts...);
}

