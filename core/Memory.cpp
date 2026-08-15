#include "Memory.h"
#include "Process.h"

namespace coco {

HANDLE Memory::ProcessHandle() const {
    return m_process ? m_process->Handle() : nullptr;
}

bool Memory::ReadRaw(uintptr_t address, void* buffer, size_t size) const {
    if (!m_process || !m_process->IsAttached()) return false;
    SIZE_T bytesRead = 0;
    return ::ReadProcessMemory(m_process->Handle(),
                               reinterpret_cast<LPCVOID>(address),
                               buffer, size, &bytesRead)
           && bytesRead == size;
}

bool Memory::WriteRaw(uintptr_t address, const void* buffer,
                      size_t size) const {
    if (!m_process || !m_process->IsAttached()) return false;
    SIZE_T written = 0;
    return ::WriteProcessMemory(m_process->Handle(),
                                reinterpret_cast<LPVOID>(address),
                                buffer, size, &written)
           && written == size;
}

uintptr_t Memory::ResolvePointerChain(
    uintptr_t baseAddress,
    const std::vector<uint32_t>& offsets) const
{
    if (!m_process || !m_process->IsAttached()) return 0;
    if (baseAddress == 0) return 0;

    uintptr_t addr = baseAddress;
    // 按目标进程位宽读取指针：32 位进程读 4 字节，64 位进程读 8 字节
    const size_t ptrSize = m_process->Is64Bit() ? sizeof(uint64_t)
                                                : sizeof(uint32_t);

    uint64_t val = 0;
    if (!ReadRaw(addr, &val, ptrSize)) return 0;

    for (size_t i = 0; i < offsets.size(); ++i) {
        addr = static_cast<uintptr_t>(val) + offsets[i];
        if (i == offsets.size() - 1) return addr;
        val = 0;
        if (!ReadRaw(addr, &val, ptrSize) || val == 0) return 0;
    }
    return static_cast<uintptr_t>(val);
}

} // namespace coco
