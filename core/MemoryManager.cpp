#include "MemoryManager.h"
#include <tlhelp32.h>

// ==============================
// 进程生命周期
// ==============================

MemoryManager::~MemoryManager() {
    DetachProcess();
}

// 按进程名查找并附加目标进程
bool MemoryManager::AttachProcess(const std::wstring& processName) {
    // 遍历系统快照获取目标进程 PID
    m_processId = GetProcessIdByName(processName);
    if (m_processId == 0) {
        return false;
    }

    // 以完全访问权限打开进程句柄
    m_processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_processId);
    return m_processHandle != nullptr;
}

void MemoryManager::DetachProcess() {
    if (m_processHandle != nullptr) {
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }
    m_processId = 0;
}

bool MemoryManager::IsAttached() const {
    return m_processHandle != nullptr;
}

// ==============================
// 通用指针链解析
// ==============================

uintptr_t MemoryManager::ResolvePointerChain(uintptr_t baseAddress, const std::vector<DWORD>& offsets) {
    if (!IsAttached()) {
        return 0;
    }

    if (baseAddress == 0) {
        return 0;
    }

    // 说明：
    // 1) currentAddress 使用 uintptr_t，是为了兼容当前编译目标位宽（x86/x64）。
    // 2) currentValue 仍使用 DWORD，是因为本项目目标游戏进程的指针链节点按 32 位地址存储。
    //    读取后再提升到 uintptr_t 参与后续地址运算，可同时兼顾语义清晰和编译器告警消除。
    uintptr_t currentAddress = baseAddress;
    DWORD currentValue = 0;

    // 读取基地址的值
    if (!ReadMemory(currentAddress, &currentValue, sizeof(DWORD))) {
        return 0;
    }

    // 遍历偏移链
    for (size_t i = 0; i < offsets.size(); ++i) {
        currentAddress = currentValue + offsets[i];

        // 如果是最后一个偏移，直接返回地址
        if (i == offsets.size() - 1) {
            return currentAddress;
        }

        // 否则继续读取下一级指针
        if (!ReadMemory(currentAddress, &currentValue, sizeof(DWORD))) {
            return 0;
        }

        // 检查空指针
        if (currentValue == 0) {
            return 0;
        }
    }

    // 如果没有偏移，返回基地址读取的值
    return currentValue;
}

// ==============================
// 阳光值读写
// ==============================

bool MemoryManager::ReadSunshine(int& sunshine) {
    if (!IsAttached()) {
        return false;
    }

    // 阳光地址: [[006A9EC0] + 0x768] + 0x5560
    return ReadPointerChainValue<int>(kBaseAddress, {kOffset1, kOffset2}, sunshine);
}

bool MemoryManager::WriteSunshine(int sunshine) {
    if (!IsAttached()) {
        return false;
    }

    // 阳光地址: [[006A9EC0] + 0x768] + 0x5560
    return WritePointerChainValue<int>(kBaseAddress, {kOffset1, kOffset2}, sunshine);
}

// ==============================
// CD 格写入
// ==============================

bool MemoryManager::WriteCDSlot(int slot, bool enabled) {
    if (!IsAttached()) {
        return false;
    }

    // CD格地址: [[[006A9EC0] + 0x768] + 0x144] + slotOffset
    // slotOffset 在每个格位之间步进 0x50 字节
    DWORD slotOffset = 0;
    switch (slot) {
    case 1:
        slotOffset = 0x70;
        break;
    case 2:
        slotOffset = 0x70 + 0x50;
        break;
    case 3:
        slotOffset = 0x70 + 0x50 + 0x50;
        break;
    default:
        return false;
    }

    BYTE value = enabled ? 1 : 0;
    return WritePointerChainValue<BYTE>(kBaseAddress, {kOffset1, 0x144, slotOffset}, value);
}

// ==============================
// 自动采集阳光（远程线程调用）
// ==============================

bool MemoryManager::CollectSunshine() {
    if (!IsAttached()) {
        return false;
    }

    // 自动采集阳光地址: [[006A9EC0] + 0x768] + 0xE4
    uintptr_t targetAddress = ResolvePointerChain(kBaseAddress, {kOffset1, 0xE4});
    if (targetAddress == 0) {
        return false;
    }
    // 目标函数参数约定为 32 位地址，这里显式收窄，避免隐式转换带来歧义
    DWORD targetAddress32 = static_cast<DWORD>(targetAddress);

    // 在目标进程中分配内存，用于传递参数
    LPVOID paramAddress = VirtualAllocEx(m_processHandle, nullptr, sizeof(DWORD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (paramAddress == nullptr) {
        return false;
    }

    // 将参数值写入刚分配的内存
    if (!WriteProcessMemory(m_processHandle, paramAddress, &targetAddress32, sizeof(DWORD), nullptr)) {
        VirtualFreeEx(m_processHandle, paramAddress, 0, MEM_RELEASE);
        return false;
    }

    // 创建远程线程，调用游戏内部采集阳光函数 0x004309D0
    HANDLE hThread = CreateRemoteThread(
        m_processHandle,
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE)0x004309D0,
        paramAddress,
        0,
        nullptr
    );

    if (hThread == nullptr) {
        VirtualFreeEx(m_processHandle, paramAddress, 0, MEM_RELEASE);
        return false;
    }

    // 等待远程线程执行完成
    WaitForSingleObject(hThread, INFINITE);

    // 清理资源
    CloseHandle(hThread);
    VirtualFreeEx(m_processHandle, paramAddress, 0, MEM_RELEASE);

    return true;
}

// ==============================
// 内部辅助：进程快照查找
// ==============================

DWORD MemoryManager::GetProcessIdByName(const std::wstring& processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32First(snapshot, &entry)) {
        do {
            // 不区分大小写比较进程名
            if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

// ==============================
// 内部辅助：内存读写
// ==============================

bool MemoryManager::ReadMemory(uintptr_t address, void* buffer, SIZE_T size) {
    SIZE_T bytesRead;
    return ReadProcessMemory(m_processHandle, reinterpret_cast<LPCVOID>(address), buffer, size, &bytesRead) && (bytesRead == size);
}

bool MemoryManager::WriteMemory(uintptr_t address, const void* buffer, SIZE_T size) {
    SIZE_T bytesWritten;
    return WriteProcessMemory(m_processHandle, reinterpret_cast<LPVOID>(address), buffer, size, &bytesWritten) && (bytesWritten == size);
}


