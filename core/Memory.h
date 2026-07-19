#pragma once

#include <cstdint>
#include <vector>
#include <type_traits>
#include <windows.h>

namespace coco {

class Process;

// ==============================
// Memory — 远程进程内存操作
// ==============================
// 职责：通用内存读写、指针链解析。
// 不含任何游戏专用偏移或逻辑。
class Memory {
public:
    explicit Memory(Process& process) : m_process(&process) {}

    // --- 指针链解析 ---
    uintptr_t ResolvePointerChain(
        uintptr_t baseAddress,
        const std::vector<uint32_t>& offsets) const;

    // --- 模板化读写 ---
    template<typename T>
    bool Read(uintptr_t address, T& outValue) const {
        static_assert(std::is_trivially_copyable_v<T>,
                      "T must be trivially copyable");
        return ReadRaw(address, &outValue, sizeof(T));
    }

    template<typename T>
    bool Write(uintptr_t address, const T& value) const {
        static_assert(std::is_trivially_copyable_v<T>,
                      "T must be trivially copyable");
        return WriteRaw(address, &value, sizeof(T));
    }

    template<typename T>
    bool ReadPointerChain(uintptr_t base,
                          const std::vector<uint32_t>& offsets,
                          T& outValue) const {
        uintptr_t addr = ResolvePointerChain(base, offsets);
        return addr != 0 && Read(addr, outValue);
    }

    template<typename T>
    bool WritePointerChain(uintptr_t base,
                           const std::vector<uint32_t>& offsets,
                           const T& value) const {
        uintptr_t addr = ResolvePointerChain(base, offsets);
        return addr != 0 && Write(addr, value);
    }

    // --- 底层句柄（用于 CreateRemoteThread 等） ---
    HANDLE ProcessHandle() const;

private:
    bool ReadRaw(uintptr_t address, void* buffer, size_t size) const;
    bool WriteRaw(uintptr_t address, const void* buffer, size_t size) const;

    Process* m_process = nullptr;
};

} // namespace coco
