#pragma once

#include <windows.h>
#include <vector>
#include <memory>

namespace coco {

class D3D11Device;
class ImGuiRenderer;
class Process;
class Memory;
struct GameFeature;
class LogBuffer;

// ==============================
// OverlayApp — 覆盖层主程序
// ==============================
class OverlayApp {
public:
    OverlayApp();
    ~OverlayApp();

    OverlayApp(const OverlayApp&) = delete;
    OverlayApp& operator=(const OverlayApp&) = delete;

    void RegisterFeature(std::unique_ptr<GameFeature> feature);
    int  Run(HINSTANCE hInstance, int nCmdShow = SW_SHOW);

    LogBuffer& Logger() { return *m_log; }

private:
    bool CreateOverlayWindow(HINSTANCE hInstance);
    void DestroyOverlayWindow(HINSTANCE hInstance);
    bool PumpMessages(bool& shouldQuit);
    void PollHotkeys();
    void DrawMenuBar();

    static LRESULT WINAPI StaticWndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);

    HWND     m_hwnd = nullptr;
    bool     m_windowVisible = true;
    bool     m_homeWasDown = false;
    bool     m_endWasDown  = false;

    std::unique_ptr<D3D11Device>        m_d3d;
    std::unique_ptr<ImGuiRenderer>       m_imgui;
    std::unique_ptr<Process>             m_process;
    std::unique_ptr<Memory>              m_memory;
    std::unique_ptr<LogBuffer>           m_log;

    std::vector<std::unique_ptr<GameFeature>> m_features;
    GameFeature* m_activeFeature = nullptr;
};

} // namespace coco
