#pragma once

// ==============================
// PlantCallA — 方案 A：每 call 动态生成完整 shellcode
// ==============================

#include "PlantCallCommon.h"

namespace coco {

// CE sequence replicated as inline x86 stub:
// push flags; push type; mov eax,row; push col; push memMgr; call plant; ret
bool PlantViaSchemeA(const Memory& mem, const PlantCallParams& params,
                     DWORD timeoutMs = 2000);

} // namespace coco
