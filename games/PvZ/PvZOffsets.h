#pragma once

#include <cstdint>

// ==============================
// PvZOffsets — Plants vs Zombies 内存偏移常量
// ==============================
// 游戏版本: 1.2.0.1073
// 所有偏移集中于此，方便版本迁移时快速调整。
// ==============================

namespace coco {
namespace pvz {

// --- 指针链基地址 ---
inline constexpr uintptr_t kBaseAddress        = 0x006A9EC0;

// --- 指针链偏移量 ---
inline constexpr uint32_t   kPtrChainMemMgr    = 0x768;   // 内存管理器
inline constexpr uint32_t   kPtrChainSunshine  = 0x5560;  // 阳光值
inline constexpr uint32_t   kPtrChainCollect   = 0xE4;    // 自动采集函数参数
inline constexpr uint32_t   kPtrChainCDBase    = 0x144;   // CD 区基地址

// --- CD 槽位间距 ---
inline constexpr uint32_t   kCDSlotStride      = 0x50;
inline constexpr uint32_t   kCDSlot1Offset     = 0x70;

// --- 函数地址 ---
inline constexpr uintptr_t  kCollectSunshineFn = 0x004309D0;
inline constexpr uintptr_t  kPlantFn           = 0x0040D120;  // 种植 Call

// --- 种植 Call 固定参数 ---
inline constexpr uint32_t   kPlantFlags        = 0xFFFFFFFF;

} // namespace pvz
} // namespace coco
