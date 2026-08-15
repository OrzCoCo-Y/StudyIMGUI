#pragma once

#include "framework/GameFeature.h"
#include "framework/LogBuffer.h"
#include <memory>

namespace coco {

class Memory;

// ==============================
// PvZFeature — 植物大战僵尸修改器
// 6-Tab 菜单框架参考 docs/menu-framework-pvz.html
// ==============================
class PvZFeature : public GameFeature {
public:
    explicit PvZFeature(LogBuffer& logger);

    const char*   GetName() const override;
    const wchar_t* GetProcessName() const override;

    void OnAttach(Memory& mem) override;
    void OnDetach() override;
    void OnUpdate(Memory& mem) override;

    // --- UI ---
    void OnRenderUI() override;

    // --- Host bridge ---
    // 宿主桥接
    void OnFrameProcessState(bool attached, DWORD pid, HANDLE handle) override;
    UiRequest ConsumeUiRequest() override;

private:
    // --- 游戏逻辑 ---
    bool WriteSunshine(Memory& mem, int value);
    bool CollectSunshineRemote(Memory& mem);

    // --- UI 框架 ---
    void RenderHeader();
    void RenderTabBar();
    void RenderSidebar();
    void RenderPane();
    void RenderStatusBar();

    void RenderVisualPane();
    void RenderAssistPane();
    void RenderNumericPane();
    void RenderProcessPane();
    void RenderSettingsPane();
    void RenderDeveloperPane();

    // --- 日志视图 ---
    void RenderLogView();

    LogBuffer& m_log;

    // 游戏数据
    int  m_sunshine        = 0;
    int  m_pendingSunshine = 0;
    bool m_sunshineDirty   = false;  // 待写入标记（支持写 0）

    bool m_cdSlot1Enabled      = false;
    bool m_cdSlot2Enabled      = false;
    bool m_cdSlot3Enabled      = false;
    bool m_autoCollectSunshine = false;
    double m_lastAutoCollect   = -1.0;  // 上次自动采集时间（秒，用于节流）

    // 宿主桥接状态
    bool      m_attached = false;
    DWORD     m_pid      = 0;
    HANDLE    m_handle   = nullptr;
    UiRequest m_uiRequest = UiRequest::None;

    // UI 状态
    int  m_uiTab = 0;               // 0=视觉 1=辅助 2=数值 3=进程 4=设置 5=开发者
    int  m_uiSub[6] = { 0, 0, 0, 0, 0, 0 };
    bool m_logAutoScroll = true;
};

} // namespace coco