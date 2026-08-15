#include "Process.h"
#include <tlhelp32.h>

namespace coco {

bool Process::Attach(const std::wstring& processName) {
    Detach();
    m_pid = FindProcessId(processName);
    if (m_pid == 0) return false;
    // 内存修改所需的最小权限集（避免 PROCESS_ALL_ACCESS 在提权进程上失败）
    const DWORD access = PROCESS_QUERY_INFORMATION |
                         PROCESS_QUERY_LIMITED_INFORMATION |
                         PROCESS_VM_READ | PROCESS_VM_WRITE |
                         PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD |
                         SYNCHRONIZE;
    m_handle = ::OpenProcess(access, FALSE, m_pid);
    return m_handle != nullptr;
}

void Process::Detach() {
    if (m_handle) { ::CloseHandle(m_handle); m_handle = nullptr; }
    m_pid = 0;
}

bool Process::IsAlive() const {
    if (!m_handle) return false;
    DWORD exitCode = 0;
    if (!::GetExitCodeProcess(m_handle, &exitCode)) return false;
    return exitCode == STILL_ACTIVE;
}

bool Process::Is64Bit() const {
    if (!m_handle) return false;
    BOOL isWow64 = FALSE;
    if (!::IsWow64Process(m_handle, &isWow64)) return false;
#ifdef _WIN64
    // 本工具为 64 位：目标为 32 位当且仅当运行在 WOW64 下
    return isWow64 == FALSE;
#else
    // 本工具为 32 位：目标为 64 位当且仅当运行在 WOW64 下
    return isWow64 == TRUE;
#endif
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
