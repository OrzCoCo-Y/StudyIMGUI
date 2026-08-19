#pragma once

// ==============================
// PlantCallDll — 方案 C：DLL 注入 + 共享内存 IPC
// ==============================

#include "PlantCallCommon.h"
#include "SharedPlant.h"

#include <string>

namespace coco {

class PlantCallDllClient {
public:
    PlantCallDllClient() = default;
    ~PlantCallDllClient();

    PlantCallDllClient(const PlantCallDllClient&) = delete;
    PlantCallDllClient& operator=(const PlantCallDllClient&) = delete;

    // Resolve CoCoPvZMod.dll next to the host executable
    // 解析与宿主 exe 同目录下的 CoCoPvZMod.dll
    static std::wstring DefaultDllPath();

    bool Inject(HANDLE process, const std::wstring& dllPath);
    void Detach();

    bool IsInjected() const { return m_injected; }
    bool IsConnected() const { return m_shared != nullptr; }

    bool Plant(uint32_t col, uint32_t row, uint32_t plantType,
               DWORD timeoutMs = 2000);

private:
    bool OpenSharedMemory(DWORD timeoutMs);
    bool WaitForCommandClear(DWORD timeoutMs);

    bool        m_injected  = false;
    HANDLE      m_process   = nullptr;
    HANDLE      m_mapFile   = nullptr;
    pvz::SharedPlantBlock* m_shared = nullptr;
};

} // namespace coco
