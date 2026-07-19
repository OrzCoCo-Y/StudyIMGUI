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

    ImGuiRenderer(const ImGuiRenderer&) = delete;
    ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;

    bool Initialize(HWND hwnd, D3D11Device& d3d);
    void Shutdown();
    void NewFrame();
    void Render();
    bool ProcessMessage(MSG* msg);

private:
    HWND m_hwnd = nullptr;
};

} // namespace coco
