#pragma once

// ==============================
// PlantCallB — 方案 B：可复用 ThreadProc stub + 参数块
// ==============================

#include "PlantCallCommon.h"

#include <windows.h>

namespace coco {

class PlantCallBSession {
public:
    PlantCallBSession() = default;
    ~PlantCallBSession();

    PlantCallBSession(const PlantCallBSession&) = delete;
    PlantCallBSession& operator=(const PlantCallBSession&) = delete;

    bool EnsureStub(HANDLE process);
    void Release();

    bool Plant(const Memory& mem, const PlantCallParams& params,
               DWORD timeoutMs = 2000);

    bool IsReady() const { return m_stubRemote != nullptr; }

private:
    HANDLE m_process     = nullptr;
    LPVOID m_stubRemote  = nullptr;
    size_t m_stubSize    = 0;
};

} // namespace coco
