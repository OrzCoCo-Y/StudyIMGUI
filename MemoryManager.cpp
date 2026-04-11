#include "MemoryManager.h"
#include <tlhelp32.h>

MemoryManager::~MemoryManager() {
    DetachProcess();
}

bool MemoryManager::AttachProcess(const std::wstring& processName) {
    processId = GetProcessIdByName(processName);
    if (processId == 0) {
        return false;
    }

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    return hProcess != NULL;
}

void MemoryManager::DetachProcess() {
    if (hProcess != NULL) {
        CloseHandle(hProcess);
        hProcess = NULL;
    }
    processId = 0;
}

bool MemoryManager::ReadSunshine(int& sunshine) {
    if (!IsAttached()) {
        return false;
    }

    DWORD address = GetSunshineAddress();
    if (address == 0) {
        return false;
    }

    return ReadMemory(address, &sunshine, sizeof(int));
}

bool MemoryManager::WriteSunshine(int sunshine) {
    if (!IsAttached()) {
        return false;
    }

    DWORD address = GetSunshineAddress();
    if (address == 0) {
        return false;
    }

    return WriteMemory(address, &sunshine, sizeof(int));
}

bool MemoryManager::IsAttached() const {
    return hProcess != NULL;
}

DWORD MemoryManager::GetProcessIdByName(const std::wstring& processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32First(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

bool MemoryManager::ReadMemory(DWORD address, void* buffer, SIZE_T size) {
    SIZE_T bytesRead;
    return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead) && (bytesRead == size);
}

bool MemoryManager::WriteMemory(DWORD address, const void* buffer, SIZE_T size) {
    SIZE_T bytesWritten;
    return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, &bytesWritten) && (bytesWritten == size);
}

DWORD MemoryManager::GetSunshineAddress() {
    DWORD baseAddress = BASE_ADDRESS;
    DWORD firstAddress;
    DWORD secondAddress;

    if (!ReadMemory(baseAddress, &firstAddress, sizeof(DWORD))) {
        return 0;
    }

    firstAddress += OFFSET_1;
    if (!ReadMemory(firstAddress, &secondAddress, sizeof(DWORD))) {
        return 0;
    }

    secondAddress += OFFSET_2;
    return secondAddress;
}