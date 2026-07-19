#include "Process.h"
#include <tlhelp32.h>

namespace coco {

bool Process::Attach(const std::wstring& processName) {
    Detach();
    m_pid = FindProcessId(processName);
    if (m_pid == 0) return false;
    m_handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pid);
    return m_handle != nullptr;
}

void Process::Detach() {
    if (m_handle) { ::CloseHandle(m_handle); m_handle = nullptr; }
    m_pid = 0;
}

DWORD Process::FindProcessId(const std::wstring& name) const {
    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(entry);

    HANDLE snap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    DWORD pid = 0;
    if (::Process32First(snap, &entry)) {
        do {
            if (::_wcsicmp(entry.szExeFile, name.c_str()) == 0) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (::Process32Next(snap, &entry));
    }
    ::CloseHandle(snap);
    return pid;
}

} // namespace coco
