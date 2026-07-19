#pragma once

#include "framework/GameFeature.h"

namespace coco {

// ==============================
// RPGFeature — RPG 游戏修改器模板
// ==============================
// TODO: 实现具体逻辑
class RPGFeature : public GameFeature {
public:
    const char* GetName() const override { return "RPG Game (TODO)"; }
    const wchar_t* GetProcessName() const override { return L"rpg_game.exe"; }

    void OnRenderUI() override;
};

} // namespace coco
