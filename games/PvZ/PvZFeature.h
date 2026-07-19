#pragma once

#include "framework/GameFeature.h"
#include "framework/LogBuffer.h"
#include <memory>

namespace coco {

class Memory;

// ==============================
// PvZFeature — 植物大战僵尸修改器
// ==============================
class PvZFeature : public GameFeature {
public:
    explicit PvZFeature(LogBuffer& logger);

    const char*  GetName() const override;
    const wchar_t* GetProcessName() const override;
    void OnAttach(Memory& mem) override;
    void OnDetach() override;
    void OnUpdate(Memory& mem) override;
    void OnRenderUI() override;

private:
    void RenderOverviewTab();
    void RenderSunshineControls();
    void RenderFeatureToggles();
    void RenderLogPanel();

    bool WriteSunshine(Memory& mem, int value);
    bool CollectSunshineRemote(Memory& mem);

    LogBuffer& m_log;

    int m_sunshine        = 0;
    int m_pendingSunshine = 0;

    bool m_cdSlot1Enabled      = false;
    bool m_cdSlot2Enabled      = false;
    bool m_cdSlot3Enabled      = false;
    bool m_autoCollectSunshine = false;
};

} // namespace coco
