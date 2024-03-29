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

void itows(int number, wchar_t* text, int size) {
    memset(text, L'\0', size * sizeof(wchar_t));
    int index = 0;
    wchar_t tmp[5] = { '\0' };

    do {
        tmp[index++] = L'0' + (number % 10);
        number /= 10;
    } while (number && index >= 0);

    for (int i = 0; i < index; i++) {
        text[i] = tmp[index - i - 1];
    }

    text[index] = L'%';
}

uint32_t abgr_to_hex(int a, int b, int g, int r) {
    uint32_t abgr = (uint32_t)a << 24 | (uint32_t)b << 16 | (uint32_t)g << 8 | uint32_t(r);
    return abgr;
}

uint8_t bgr2gray(uint8_t b, uint8_t g, uint8_t r) {
    return 0.299 * r + 0.587 * g + 0.114 * b;
}