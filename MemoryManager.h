#pragma once

#include <windows.h>
#include <string>

class MemoryManager {
private:
    HANDLE hProcess = NULL;
    DWORD processId = 0;
    const DWORD BASE_ADDRESS = 0x006A9EC0;
    const DWORD OFFSET_1 = 0x768;
    const DWORD OFFSET_2 = 0x5560;

public:
    ~MemoryManager();
    bool AttachProcess(const std::wstring& processName);
    void DetachProcess();
    bool ReadSunshine(int& sunshine);
    bool WriteSunshine(int sunshine);
    bool IsAttached() const;

private:
    DWORD GetProcessIdByName(const std::wstring& processName);
    bool ReadMemory(DWORD address, void* buffer, SIZE_T size);
    bool WriteMemory(DWORD address, const void* buffer, SIZE_T size);
    DWORD GetSunshineAddress();
};