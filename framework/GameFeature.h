#pragma once

#include <string>
#include <vector>
#include <windows.h>

namespace coco {

class Memory;

// ==============================
// GameFeature — 游戏修改器插件接口
// ==============================
struct GameFeature {
    virtual ~GameFeature() = default;

    virtual const char*  GetName() const = 0;
    virtual const wchar_t* GetProcessName() const = 0;

    virtual void OnInit() {}
    virtual void OnAttach(Memory& mem) { (void)mem; }
    virtual void OnDetach() {}

    virtual void OnUpdate(Memory& mem) { (void)mem; }
    virtual void OnRenderUI() {}
    virtual void OnRenderMenuBar() {}
};

} // namespace coco
