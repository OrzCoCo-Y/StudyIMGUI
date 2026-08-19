#include "PlantCallCommon.h"

namespace coco {

void AppendU32LE(std::vector<uint8_t>& buf, uint32_t value) {
    buf.push_back(static_cast<uint8_t>(value & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void AppendCallRel32(std::vector<uint8_t>& code, uintptr_t target,
                     uintptr_t codeBase, size_t callInsnOffset) {
    const uint32_t rel = static_cast<uint32_t>(
        target - (codeBase + callInsnOffset + 5));
    AppendU32LE(code, rel);
}

bool ResolvePlantMemMgr(const Memory& mem, uint32_t& outMemMgr) {
    using namespace pvz;

    const uintptr_t slot = mem.ResolvePointerChain(
        kBaseAddress, {kPtrChainMemMgr});
    if (slot == 0) return false;

    if (!mem.Read(slot, outMemMgr) || outMemMgr == 0)
        return false;
    return true;
}

bool FillPlantParams(const Memory& mem, PlantCallParams& params) {
    return ResolvePlantMemMgr(mem, params.memMgr);
}

bool ExecuteRemoteCode(HANDLE process, const void* code, size_t codeSize,
                       void* param, DWORD timeoutMs, DWORD* outExitCode) {
    if (!process || !code || codeSize == 0) return false;

    LPVOID remoteCode = ::VirtualAllocEx(
        process, nullptr, codeSize,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteCode) return false;

    bool ok = false;
    SIZE_T written = 0;
    if (::WriteProcessMemory(process, remoteCode, code, codeSize, &written)
        && written == codeSize) {
        HANDLE thread = ::CreateRemoteThread(
            process, nullptr, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(remoteCode),
            param, 0, nullptr);
        if (thread) {
            const DWORD waitResult = ::WaitForSingleObject(thread, timeoutMs);
            if (waitResult == WAIT_OBJECT_0 && outExitCode) {
                ::GetExitCodeThread(thread, outExitCode);
            }
            ::CloseHandle(thread);
            ok = (waitResult == WAIT_OBJECT_0);
        }
    }

    ::VirtualFreeEx(process, remoteCode, 0, MEM_RELEASE);
    return ok;
}

} // namespace coco
