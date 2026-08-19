#include "PlantCallB.h"

namespace coco {
namespace {

// ThreadProc(LPVOID param) — reads PlantCallParams from remote memory
// 通用 stub：从远程参数块读取 memMgr/col/row/type 并 call 种植函数
std::vector<uint8_t> BuildReusablePlantStub(uintptr_t stubBase) {
    using namespace pvz;

    std::vector<uint8_t> code;
    code.reserve(48);

    // push ebp / mov ebp, esp
    code.push_back(0x55);
    code.insert(code.end(), {0x8B, 0xEC});

    // mov esi, [ebp+8]  — param pointer
    code.insert(code.end(), {0x8B, 0x75, 0x08});

    // push [esi+16] flags
    code.insert(code.end(), {0xFF, 0x76, 0x10});
    // push [esi+12] plantType
    code.insert(code.end(), {0xFF, 0x76, 0x0C});
    // mov eax, [esi+8] row
    code.insert(code.end(), {0x8B, 0x46, 0x08});
    // push [esi+4] col
    code.insert(code.end(), {0xFF, 0x76, 0x04});
    // push [esi+0] memMgr
    code.insert(code.end(), {0xFF, 0x36});

    const size_t callOffset = code.size();
    code.push_back(0xE8);
    AppendCallRel32(code, kPlantFn, stubBase, callOffset);

    // xor eax, eax / pop ebp / ret 4
    code.insert(code.end(), {0x33, 0xC0});
    code.push_back(0x5D);
    code.insert(code.end(), {0xC2, 0x04, 0x00});

    return code;
}

} // namespace

PlantCallBSession::~PlantCallBSession() {
    Release();
}

void PlantCallBSession::Release() {
    if (m_process && m_stubRemote) {
        ::VirtualFreeEx(m_process, m_stubRemote, 0, MEM_RELEASE);
    }
    m_stubRemote = nullptr;
    m_stubSize   = 0;
    m_process    = nullptr;
}

bool PlantCallBSession::EnsureStub(HANDLE process) {
    if (!process) return false;
    if (m_stubRemote && m_process == process) return true;

    Release();
    m_process = process;

    constexpr size_t kStubAlloc = 64;
    m_stubRemote = ::VirtualAllocEx(
        process, nullptr, kStubAlloc,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!m_stubRemote) {
        m_process = nullptr;
        return false;
    }

    const uintptr_t stubBase = reinterpret_cast<uintptr_t>(m_stubRemote);
    const std::vector<uint8_t> stub = BuildReusablePlantStub(stubBase);
    m_stubSize = stub.size();

    SIZE_T written = 0;
    if (!::WriteProcessMemory(process, m_stubRemote, stub.data(),
                              stub.size(), &written)
        || written != stub.size()) {
        Release();
        return false;
    }
    return true;
}

bool PlantCallBSession::Plant(const Memory& mem, const PlantCallParams& params,
                              DWORD timeoutMs) {
    HANDLE process = mem.ProcessHandle();
    if (!process || params.memMgr == 0) return false;
    if (!EnsureStub(process)) return false;

    LPVOID remoteParams = ::VirtualAllocEx(
        process, nullptr, sizeof(PlantCallParams),
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteParams) return false;

    bool ok = false;
    SIZE_T written = 0;
    if (::WriteProcessMemory(process, remoteParams, &params,
                             sizeof(params), &written)
        && written == sizeof(params)) {
        HANDLE thread = ::CreateRemoteThread(
            process, nullptr, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(m_stubRemote),
            remoteParams, 0, nullptr);
        if (thread) {
            const DWORD waitResult = ::WaitForSingleObject(thread, timeoutMs);
            ::CloseHandle(thread);
            ok = (waitResult == WAIT_OBJECT_0);
        }
    }

    ::VirtualFreeEx(process, remoteParams, 0, MEM_RELEASE);
    return ok;
}

} // namespace coco
