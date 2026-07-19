#pragma once

// ==============================
// CoCo Core — 公共类型与前向声明
// ==============================

#include <cstdint>
#include <windows.h>

namespace coco {

// 日志级别
enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

// D3D11 设备前向声明
class D3D11Device;
class ImGuiRenderer;
class Process;
class Memory;

} // namespace coco
