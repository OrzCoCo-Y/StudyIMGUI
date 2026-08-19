#pragma once

// ==============================
// PlantCallCommon — 种植 Call 公共类型与辅助
// ==============================

#include "PvZOffsets.h"
#include "core/Memory.h"

#include <cstdint>
#include <vector>

namespace coco {

struct PlantCallParams {
    uint32_t memMgr    = 0;  // 运行时指针，如 0x1151A508
    uint32_t col       = 0;
    uint32_t row       = 3;
    uint32_t plantType = 4;
    uint32_t flags     = pvz::kPlantFlags;
};

// Resolve the memMgr pointer pushed in CE scripts ([[base]+768])
// 解析 CE 中 push 的对象指针（[[基址]+768] 处读 dword）
bool ResolvePlantMemMgr(const Memory& mem, uint32_t& outMemMgr);

bool FillPlantParams(const Memory& mem, PlantCallParams& params);

// Remote execution helpers shared by scheme A / B
// 方案 A/B 共用的远程执行辅助
bool ExecuteRemoteCode(HANDLE process, const void* code, size_t codeSize,
                       void* param, DWORD timeoutMs, DWORD* outExitCode);

void AppendU32LE(std::vector<uint8_t>& buf, uint32_t value);
void AppendCallRel32(std::vector<uint8_t>& code, uintptr_t target,
                     uintptr_t codeBase, size_t callInsnOffset);

} // namespace coco
