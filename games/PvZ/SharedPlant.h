#pragma once

// ==============================
// SharedPlant — CoCoPvZ.exe 与 CoCoPvZMod.dll 共享 IPC
// ==============================

#include <windows.h>
#include <cstdint>

namespace coco {
namespace pvz {

inline constexpr wchar_t kSharedMemName[] = L"Local\\CoCoPvZMod_v1";

enum class PlantCommand : uint32_t {
    None     = 0,
    Plant    = 1,
    Shutdown = 2,
};

// Mapped into both processes after DLL injection
// 注入后由 DLL 创建、外部工具打开同一块共享内存
struct SharedPlantBlock {
    volatile PlantCommand command = PlantCommand::None;
    volatile uint32_t     result  = 0;  // 1 = success, 0 = failure

    uint32_t col       = 0;
    uint32_t row       = 0;
    uint32_t plantType = 4;
};

} // namespace pvz
} // namespace coco
