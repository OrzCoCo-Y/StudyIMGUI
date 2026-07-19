#include "ImGuiRenderer.h"
#include "D3D11Device.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace coco {

ImGuiRenderer::~ImGuiRenderer() { Shutdown(); }

bool ImGuiRenderer::Initialize(HWND hwnd, D3D11Device& d3d) {
    m_hwnd = hwnd;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\simsun.ttc", 18.0f, nullptr,
        io.Fonts->GetGlyphRangesChineseFull());

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(hwnd)) return false;
    if (!ImGui_ImplDX11_Init(d3d.Device(), d3d.Context())) {
        ImGui_ImplWin32_Shutdown();
        return false;
    }
    return true;
}

void ImGuiRenderer::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiRenderer::NewFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiRenderer::Render() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool ImGuiRenderer::ProcessMessage(MSG* msg) {
    return ImGui_ImplWin32_WndProcHandler(
        m_hwnd, msg->message, msg->wParam, msg->lParam) != 0;
}

} // namespace coco
