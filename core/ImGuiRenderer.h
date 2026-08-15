#pragma once

#include "imgui.h"
#include <windows.h>

namespace coco {

class D3D11Device;

// ==============================
// ImGuiRenderer — ImGui 上下文管理
// ==============================
class ImGuiRenderer {
public:
    ImGuiRenderer() = default;
    ~ImGuiRenderer();

    // The overlay app owns exactly one renderer; features fetch fonts through it.
    static ImGuiRenderer* Instance() { return s_instance; }

    ImGuiRenderer(const ImGuiRenderer&) = delete;
    ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;

    bool Initialize(HWND hwnd, D3D11Device& d3d);
    void Shutdown();
    void NewFrame();
    void Render();
    bool ProcessMessage(MSG* msg);

    // --- Fonts (Segoe UI / Microsoft YaHei / Consolas) ---
    ImFont* FontUi()    const { return m_fontUi; }     // 17px brand
    ImFont* FontBody()  const { return m_fontBody; }   // 16px body / navigation
    ImFont* FontSmall() const { return m_fontSmall; }  // 15px controls / descriptions
    ImFont* FontTiny()  const { return m_fontTiny; }   // 13px badges / status
    ImFont* FontMicro() const { return m_fontMicro; }  // 11px compact labels
    ImFont* FontMono()  const { return m_fontMono; }   // 14px PID / addresses
    ImFont* FontTitle() const { return m_fontTitle; }  // 18px pane title

private:
    static ImGuiRenderer* s_instance;

    HWND m_hwnd = nullptr;
    bool m_contextCreated = false;
    bool m_win32Initialized = false;
    bool m_dx11Initialized = false;
    ImFont* m_fontUi    = nullptr;
    ImFont* m_fontBody  = nullptr;
    ImFont* m_fontSmall = nullptr;
    ImFont* m_fontTiny  = nullptr;
    ImFont* m_fontMicro = nullptr;
    ImFont* m_fontMono  = nullptr;
    ImFont* m_fontTitle = nullptr;
};

} // namespace coco
