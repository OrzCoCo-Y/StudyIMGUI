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

    // ==============================
    // Host bridge
    // 宿主桥接
    // ==============================

    // Process operation requested by the feature UI (attach / detach / reconnect)
    // 功能 UI 向宿主请求的进程操作（附加 / 分离 / 重连）
    enum class UiRequest { None, Attach, Detach, Reconnect };

    // Returns and consumes a single UI request, called once per frame
    // 返回并消费一个 UI 请求，每帧调用一次
    virtual UiRequest ConsumeUiRequest() { return UiRequest::None; }

    // Pushes the host process state every frame (attach status / PID / handle)
    // 每帧推送宿主进程状态（附加状态 / PID / 句柄）
    virtual void OnFrameProcessState(bool attached, DWORD pid, HANDLE handle) {
        (void)attached;
        (void)pid;
        (void)handle;
    }
};

} // namespace coco