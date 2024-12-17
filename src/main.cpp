#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include "offsets.h"

DWORD_PTR GetModuleBaseAddress(DWORD pid) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W mod = { sizeof(mod) };
        if (Module32FirstW(snap, &mod)) {
            CloseHandle(snap);
            return (DWORD_PTR)mod.modBaseAddr;
        }
    }
    CloseHandle(snap);
    return 0;
}

int main() {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe = { sizeof(pe) };
        for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe))
            if (_wcsicmp(pe.szExeFile, L"ac_client.exe") == 0) { pid = pe.th32ProcessID; break; }
        CloseHandle(snap);
    }
    if (!pid || !(snap = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, 0, pid))) return 1;

    DWORD_PTR base = GetModuleBaseAddress(pid), hp, ap, pp;
    const int Val = 9999;

    while (true) {
        ReadProcessMemory(snap, (LPCVOID)(base + offsets::health_ptr), &hp, sizeof(hp), 0);
        ReadProcessMemory(snap, (LPCVOID)(base + offsets::ammo_ptr), &ap, sizeof(ap), 0);
        ReadProcessMemory(snap, (LPCVOID)(base + offsets::ammo_ptr), &pp, sizeof(pp), 0);

        WriteProcessMemory(snap, (LPVOID)(hp + offsets::health_off), &Val, sizeof(Val), 0);
        WriteProcessMemory(snap, (LPVOID)(ap + offsets::assault_rifle_off), &Val, sizeof(Val), 0);
        WriteProcessMemory(snap, (LPVOID)(pp + offsets::pistol_off), &Val, sizeof(Val), 0);

        printf("Set Health, AR and Pistol Ammo to 9999");
        Sleep(100);
        break;
    }

    CloseHandle(snap);
    return 0;
}