#include "pch.h"
#include <Psapi.h>
#include <algorithm>
#include <vector>
#include<sstream>

MODULEINFO get_module_info(const char* szModule) {
    MODULEINFO modinfo = { 0 };
    HMODULE hModule = GetModuleHandleA(szModule);
    if (hModule == 0) return modinfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
    return modinfo;
}

void shellcode_write(PVOID ptr, PVOID byte, SIZE_T size) {
    DWORD cur, tmp;
    VirtualProtect((PVOID)ptr, size, PAGE_EXECUTE_READWRITE, &cur);
    memset((PVOID)ptr, 0x90, size);
    memcpy((PVOID)ptr, byte, size);
    VirtualProtect((PVOID)ptr, size, cur, &tmp);
}

template <typename Retn>
Retn* address_offset(QWORD base, QWORD offset) {
    return (Retn*)(*(QWORD*)base + offset);
}

template <typename Retn, typename ... Ts>
Retn* address_offset(QWORD base, QWORD offset, Ts ... ts) {
    return address_offset(*(QWORD*)base + offset, ts...);
}

UINT64 find_pattern(MODULEINFO* hmodule, std::string pattern) {
    std::transform(pattern.begin(), pattern.end(), pattern.begin(), toupper);
    std::istringstream iss(pattern);
    std::vector<int16_t> address_arr;
    std::string tmp;

    while (std::getline(iss, tmp, ' ')) {
        if (tmp != "??")
            address_arr.push_back(stoi(tmp, 0, 16));
        else
            address_arr.push_back(-1);
    }

    BYTE* lp_buffer = (BYTE*)((UINT64)hmodule->lpBaseOfDll);
    for (uint64_t i = 0; i < hmodule->SizeOfImage; i++) {
        bool found = true;
        for (uint64_t j = 0; j < address_arr.size(); ++j) {
            if (lp_buffer[i + j] != address_arr[j] && address_arr[j] != -1) {
                found = false;
                break;
            }
        }
        if (found) {
            return (UINT64)hmodule->lpBaseOfDll + i;
        }
    }
    return 0;
}