#pragma once

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include <d3d11.h>
#include <string>
#include <vector>

// ==============================
// ImGui 管理器
// CoCo: Encapsulates Dear ImGui + DirectX 11 init, frame loop & cleanup
// 封装 Dear ImGui + DirectX 11 渲染管线的初始化、帧循环与资源清理，
// Provides CoCo modifier UI (sunshine, CD toggles, auto-collect)
// 并提供游戏修改器主界面（阳光修改、CD 格开关、自动采集）的 UI 逻辑。
// ==============================
class ImGuiManager {
private:
    // D3D11 Resources
    // --- D3D11 渲染资源 ---
    ID3D11Device* m_pd3dDevice = nullptr;               // D3D11 设备
    ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;  // D3D11 设备上下文
    IDXGISwapChain* m_pSwapChain = nullptr;              // 交换链
    ID3D11RenderTargetView* m_mainRenderTargetView = nullptr; // 主渲染目标视图
    HWND m_hwnd = nullptr;                               // 覆盖层窗口句柄

    // Feature Toggle States (per-frame locked, like CE)
    // --- 功能开关状态（每帧持续写入，类似 CE 锁定） ---
    bool m_cdSlot1Enabled = false;  // 1 格无 CD
    bool m_cdSlot2Enabled = false;  // 2 格无 CD
    bool m_cdSlot3Enabled = false;  // 3 格无 CD
    bool m_autoCollectSunshine = false;  // 自动采集阳光

    // Log
    // --- 日志 ---
    std::vector<std::string> m_logMessages;  // 带时间戳的日志条目，最多 100 条

public:
    // D3D11 device
    // 初始化 ImGui 上下文、D3D11 设备与渲染后端
    bool Initialize(HWND hWnd);
    // Shutdown ImGui context & release D3D11 resources
    // 销毁 ImGui 上下文并释放 D3D11 资源
    void Shutdown();
    // Start new frame (DX11 -> Win32 -> ImGui NewFrame)
    // 开始新帧（依次调用 DX11 / Win32 / ImGui NewFrame）
    void NewFrame();
    // Submit ImGui draw commands and Present to screen
    // 提交 ImGui 绘制命令并 Present 到屏幕
    void Render();
    // Forward Win32 messages to ImGui (input, scroll, etc.)
    // 将 Win32 消息转发给 ImGui 内部处理（输入、滚轮等）
    bool ProcessMessage(MSG* msg);
    // Log
    // 渲染主功能窗口（标签页 + 持续功能 + 日志面板）
    void RenderSunshineWindow(int* sunshine, int& pendingSunshine);
    // Handle window resize, rebuild render target
    // 响应窗口尺寸变化，重建渲染目标
    void HandleResize(WPARAM wParam, LPARAM lParam);
    // Log
    // 向日志列表追加一条带时间戳的消息
    void AddLog(const std::string& message);

private:
    // 渲染"概览"标签页：当前阳光值与进程状态
    void RenderOverviewTab(const int& sunshine);
    // 渲染"修改"标签页：阳光输入控件 + 功能开关
    void RenderModifyTab(const int& sunshine, int& pendingSunshine);
    // 渲染阳光值输入与"应用修改"按钮
    void RenderSunshineControls(const int& sunshine, int& pendingSunshine);
    // Auto collect sunshine
    // 渲染 CD 格开关与自动采集阳光复选框
    void RenderFeatureToggles();
    // Render connection status (green/red + reconnect)
    // 渲染进程连接状态（已连接绿色 / 未连接红色 + 重连按钮）
    void RenderProcessStatus();
    // Log
    // 将日志列表写入文件
    bool SaveLogsToFile(const char* path);
    // Log
    // 渲染日志面板（清空/保存按钮 + 可滚动日志区域）
    void RenderLogPanel();
    // Per-frame: write features to game based on toggle states
    // 每帧根据开关状态持续向游戏进程写入功能数据（CD 锁定、自动采集）
    void ApplyContinuousFeatures();

    // D3D11 Helper Methods
    // --- D3D11 辅助方法 ---
    bool CreateDeviceD3D(HWND hWnd);               // 创建 D3D11 设备与交换链
    void CleanupDeviceD3D();                        // 释放 D3D11 设备与交换链
    void CreateRenderTarget();                      // 从交换链后台缓冲区创建渲染目标视图
    void CleanupRenderTarget();                     // 释放渲染目标视图
};

