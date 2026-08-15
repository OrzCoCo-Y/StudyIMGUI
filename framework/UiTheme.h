#pragma once

// ==============================
// UiTheme — 共享 UI 主题（调色板 + 标题栏布局常量）
// 调色板与 docs/menu-framework-pvz.html 设计稿一致，
// 供 OverlayApp（自定义标题栏/窗口控制按钮）与各 Feature UI 共用。
// ==============================

#include "imgui.h"

namespace coco {

// --- 调色板 Palette ---
inline constexpr ImVec4 kWindowBg   (0.102f, 0.102f, 0.141f, 1.0f);  // #1a1a24
inline constexpr ImVec4 kSurfaceBg  (0.133f, 0.133f, 0.180f, 1.0f);  // #22222e
inline constexpr ImVec4 kHoverBg    (0.173f, 0.173f, 0.227f, 1.0f);  // #2c2c3a
inline constexpr ImVec4 kActiveBg   (0.208f, 0.208f, 0.290f, 1.0f);  // #35354a
inline constexpr ImVec4 kBorder     (0.255f, 0.263f, 0.345f, 1.0f);  // #414358
inline constexpr ImVec4 kText       (0.910f, 0.918f, 0.965f, 1.0f);  // #e8eaf6
inline constexpr ImVec4 kTextDim    (0.635f, 0.651f, 0.733f, 1.0f);  // #a2a6bb
inline constexpr ImVec4 kTextBright (0.941f, 0.941f, 1.000f, 1.0f);  // #f0f0ff
inline constexpr ImVec4 kAccent     (0.424f, 0.549f, 1.000f, 1.0f);  // #6c8cff
inline constexpr ImVec4 kGreen      (0.435f, 0.812f, 0.592f, 1.0f);  // #6fcf97
inline constexpr ImVec4 kRed        (0.922f, 0.341f, 0.341f, 1.0f);  // #eb5757
inline constexpr ImVec4 kOrange     (0.949f, 0.600f, 0.290f, 1.0f);  // #f2994a

inline constexpr ImVec4 kAccentBg(kAccent.x, kAccent.y, kAccent.z, 0.10f);
inline constexpr ImVec4 kHoverAccent(kAccent.x, kAccent.y, kAccent.z, 0.18f);
inline constexpr ImVec4 kActiveAccent(kAccent.x, kAccent.y, kAccent.z, 0.24f);

inline ImU32 ToU32(const ImVec4& color) {
    return ImGui::ColorConvertFloat4ToU32(color);
}

// --- 自定义标题栏布局 ---
inline constexpr float kTitleBarHeight = 42.0f;  // 标题栏高度（px）
inline constexpr float kWinCtlWidth    = 40.0f;  // 单个窗口控制按钮宽度（px）
inline constexpr int   kWinCtlCount    = 3;      // 最小化 / 最大化 / 关闭
inline constexpr float kWinCtlGap      = 2.0f;   // 控制按钮间距（px）

} // namespace coco
