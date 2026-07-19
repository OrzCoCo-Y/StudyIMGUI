#pragma once

#include "framework/GameFeature.h"

namespace coco {

// ==============================
// FPSFeature — FPS 游戏修改器模板
// ==============================
// TODO: 实现具体逻辑
class FPSFeature : public GameFeature {
public:
    const char* GetName() const override { return "FPS Game (TODO)"; }
    const wchar_t* GetProcessName() const override { return L"fps_game.exe"; }

    void OnRenderUI() override;
};

} // namespace coco
