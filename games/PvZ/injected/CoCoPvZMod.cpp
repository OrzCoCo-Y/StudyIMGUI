// ==============================
// CoCoPvZMod.dll — 方案 C：注入到 PvZ 进程内执行种植 Call
// ==============================

#include "../SharedPlant.h"
#include "../PvZOffsets.h"

#include <windows.h>
#include <cstdint>
#include <cstring>

namespace {

using namespace coco::pvz;

HANDLE g_mapFile = nullptr;
SharedPlantBlock* g_shared = nullptr;
HANDLE g_workerThread = nullptr;
volatile bool g_running = false;

uint32_t ResolveMemMgrLocal() {
    __try {
        const uint32_t* base = reinterpret_cast<const uint32_t*>(kBaseAddress);
        const uint32_t root = *base;
        if (root == 0) return 0;
        const uint32_t* slot = reinterpret_cast<const uint32_t*>(
            static_cast<uintptr_t>(root) + kPtrChainMemMgr);
        return *slot;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

using PlantShellcodeFn = void(__stdcall*)();

bool ExecutePlantShellcode(const uint8_t* code, size_t size) {
    __try {
        auto fn = reinterpret_cast<PlantShellcodeFn>(
            const_cast<uint8_t*>(code));
        fn();
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool PlantInProcess(uint32_t col, uint32_t row, uint32_t plantType) {
    const uint32_t memMgr = ResolveMemMgrLocal();
    if (memMgr == 0) return false;

    LPVOID exec = ::VirtualAlloc(
        nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!exec) return false;

    const uintptr_t codeBase = reinterpret_cast<uintptr_t>(exec);
    uint8_t code[40]{};
    size_t pos = 0;

    auto pushImm32 = [&](uint32_t value) {
        code[pos++] = 0x68;
        code[pos++] = static_cast<uint8_t>(value & 0xFF);
        code[pos++] = static_cast<uint8_t>((value >> 8) & 0xFF);
        code[pos++] = static_cast<uint8_t>((value >> 16) & 0xFF);
        code[pos++] = static_cast<uint8_t>((value >> 24) & 0xFF);
    };

    pushImm32(kPlantFlags);
    pushImm32(plantType);
    code[pos++] = 0xB8;
    code[pos++] = static_cast<uint8_t>(row & 0xFF);
    code[pos++] = static_cast<uint8_t>((row >> 8) & 0xFF);
    code[pos++] = static_cast<uint8_t>((row >> 16) & 0xFF);
    code[pos++] = static_cast<uint8_t>((row >> 24) & 0xFF);
    pushImm32(col);
    pushImm32(memMgr);

    const size_t callOffset = pos;
    code[pos++] = 0xE8;
    const uint32_t rel = static_cast<uint32_t>(
        kPlantFn - (codeBase + callOffset + 5));
    code[pos++] = static_cast<uint8_t>(rel & 0xFF);
    code[pos++] = static_cast<uint8_t>((rel >> 8) & 0xFF);
    code[pos++] = static_cast<uint8_t>((rel >> 16) & 0xFF);
    code[pos++] = static_cast<uint8_t>((rel >> 24) & 0xFF);
    code[pos++] = 0xC3;

    ::memcpy(exec, code, pos);
    const bool ok = ExecutePlantShellcode(
        static_cast<const uint8_t*>(exec), pos);

    ::VirtualFree(exec, 0, MEM_RELEASE);
    return ok;
}

DWORD WINAPI WorkerThread(LPVOID) {
    while (g_running && g_shared) {
        const PlantCommand cmd = g_shared->command;
        if (cmd == PlantCommand::Plant) {
            const bool ok = PlantInProcess(
                g_shared->col, g_shared->row, g_shared->plantType);
            g_shared->result  = ok ? 1u : 0u;
            g_shared->command = PlantCommand::None;
        } else if (cmd == PlantCommand::Shutdown) {
            g_running = false;
            break;
        }
        ::Sleep(5);
    }
    return 0;
}

void CleanupShared() {
    g_running = false;
    if (g_workerThread) {
        ::WaitForSingleObject(g_workerThread, 2000);
        ::CloseHandle(g_workerThread);
        g_workerThread = nullptr;
    }
    if (g_shared) {
        ::UnmapViewOfFile(g_shared);
        g_shared = nullptr;
    }
    if (g_mapFile) {
        ::CloseHandle(g_mapFile);
        g_mapFile = nullptr;
    }
}

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            ::DisableThreadLibraryCalls(hModule);

            g_mapFile = ::CreateFileMappingW(
                INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                sizeof(SharedPlantBlock), kSharedMemName);
            if (!g_mapFile) return FALSE;

            g_shared = static_cast<SharedPlantBlock*>(::MapViewOfFile(
                g_mapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedPlantBlock)));
            if (!g_shared) {
                CleanupShared();
                return FALSE;
            }

            *g_shared = SharedPlantBlock{};
            g_running = true;
            g_workerThread = ::CreateThread(
                nullptr, 0, WorkerThread, nullptr, 0, nullptr);
            if (!g_workerThread) {
                CleanupShared();
                return FALSE;
            }
            break;

        case DLL_PROCESS_DETACH:
            CleanupShared();
            break;

        default:
            break;
    }
    return TRUE;
}
