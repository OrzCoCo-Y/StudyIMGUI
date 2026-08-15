#pragma once

#include <windows.h>
#include <string>

namespace coco {

// ==============================
// Process — 目标进程生命周期管理
// ==============================
// 职责：按名称查找进程、打开/关闭句柄。
// 不包含任何地址或偏移信息。
class Process {
public:
    Process() = default;
    ~Process() { Detach(); }

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    // 按进程名查找并打开句柄
    bool Attach(const std::wstring& processName);

    // 关闭句柄，重置状态
    void Detach();

    bool   IsAttached() const { return m_handle != nullptr; }
    HANDLE Handle()     const { return m_handle; }
    DWORD  ProcessId()  const { return m_pid; }

    // 目标进程是否仍在运行（用于检测退出）
    bool IsAlive() const;

    // 目标进程是否为 64 位（用于指针链解析位宽）
    bool Is64Bit() const;

private:
    DWORD  FindProcessId(const std::wstring& name) const;

    HANDLE m_handle = nullptr;
    DWORD  m_pid    = 0;
};

} // namespace coco
