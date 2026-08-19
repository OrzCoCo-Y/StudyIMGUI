#include "PlantCallA.h"

namespace coco {
namespace {

std::vector<uint8_t> BuildInlinePlantShellcode(const PlantCallParams& params,
                                               uintptr_t codeBase) {
    using namespace pvz;

    std::vector<uint8_t> code;
    code.reserve(40);

    auto pushImm32 = [&](uint32_t value) {
        code.push_back(0x68);
        AppendU32LE(code, value);
    };

    pushImm32(params.flags);
    pushImm32(params.plantType);
    code.push_back(0xB8);
    AppendU32LE(code, params.row);
    pushImm32(params.col);
    pushImm32(params.memMgr);

    const size_t callOffset = code.size();
    code.push_back(0xE8);
    AppendCallRel32(code, kPlantFn, codeBase, callOffset);

    code.push_back(0xC3);
    return code;
}

} // namespace

bool PlantViaSchemeA(const Memory& mem, const PlantCallParams& params,
                     DWORD timeoutMs) {
    HANDLE process = mem.ProcessHandle();
    if (!process || params.memMgr == 0) return false;

    constexpr size_t kAllocSize = 64;
    LPVOID remoteCode = ::VirtualAllocEx(
        process, nullptr, kAllocSize,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteCode) return false;

    const uintptr_t codeBase = reinterpret_cast<uintptr_t>(remoteCode);
    const std::vector<uint8_t> shellcode =
        BuildInlinePlantShellcode(params, codeBase);

    bool ok = false;
    SIZE_T written = 0;
    if (::WriteProcessMemory(process, remoteCode, shellcode.data(),
                             shellcode.size(), &written)
        && written == shellcode.size()) {
        HANDLE thread = ::CreateRemoteThread(
            process, nullptr, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(remoteCode),
            nullptr, 0, nullptr);
        if (thread) {
            const DWORD waitResult = ::WaitForSingleObject(thread, timeoutMs);
            ::CloseHandle(thread);
            ok = (waitResult == WAIT_OBJECT_0);
        }
    }

    ::VirtualFreeEx(process, remoteCode, 0, MEM_RELEASE);
    return ok;
}

} // namespace coco
