#include "ImGuiRenderer.h"
#include "D3D11Device.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace coco {

ImGuiRenderer* ImGuiRenderer::s_instance = nullptr;


ImGuiRenderer::~ImGuiRenderer() { Shutdown(); }

bool ImGuiRenderer::Initialize(HWND hwnd, D3D11Device& d3d) {
    Shutdown();
    s_instance = this;
    m_hwnd = hwnd;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_contextCreated = true;
    ImGuiIO& io = ImGui::GetIO();

    // Font stack mirrors docs/menu-framework-pvz.html:
    //   --font: 'Segoe UI','Microsoft YaHei',sans-serif; --mono: Consolas
    // 1.92 dynamic atlas: fonts are built/rasterized automatically on first use,
    // so multiple fonts may share one atlas without manual texture upload.
    const ImWchar* ranges = io.Fonts->GetGlyphRangesChineseFull();
    ImFontConfig cfg;
    cfg.OversampleH = 2;
    cfg.OversampleV = 2;

    // Keep icons in the BMP and merge Segoe UI Symbol so every tab and submenu
    // gets a stable monochrome glyph without depending on COLR emoji support.
    static const ImWchar kSymbolRanges[] = {
        0x2100, 0x23FF,   // Letterlike, arrows, math and technical symbols
        0x25A0, 0x27BF,   // Geometric shapes and miscellaneous symbols
        0,
    };

    // Larger defaults keep Chinese text crisp and readable on a standard
    // Windows desktop window instead of the former full-screen overlay.
    m_fontUi = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", 17.0f, &cfg, ranges);
    cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 17.0f, &cfg, ranges);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 17.0f, &cfg, kSymbolRanges);
    cfg.MergeMode = false;

    m_fontBody = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", 16.0f, &cfg, ranges);
    cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 16.0f, &cfg, ranges);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 16.0f, &cfg, kSymbolRanges);
    cfg.MergeMode = false;

    m_fontTiny = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", 13.0f, &cfg, ranges);
    cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 13.0f, &cfg, ranges);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 13.0f, &cfg, kSymbolRanges);
    cfg.MergeMode = false;

    m_fontMicro = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", 13.0f, &cfg, ranges);
    cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 13.0f, &cfg, ranges);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 13.0f, &cfg, kSymbolRanges);
    cfg.MergeMode = false;

    m_fontSmall = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", 15.0f, &cfg, ranges);
    cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 15.0f, &cfg, ranges);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 15.0f, &cfg, kSymbolRanges);
    cfg.MergeMode = false;

    m_fontMono = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\consola.ttf", 14.0f, &cfg, ranges);
    cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 14.0f, &cfg, ranges);
    cfg.MergeMode = false;

    m_fontTitle = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\msyhbd.ttc", 18.0f, &cfg, ranges);
    cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 18.0f, &cfg, kSymbolRanges);
    cfg.MergeMode = false;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(hwnd)) {
        Shutdown();
        return false;
    }
    m_win32Initialized = true;
    if (!ImGui_ImplDX11_Init(d3d.Device(), d3d.Context())) {
        Shutdown();
        return false;
    }
    m_dx11Initialized = true;
    return true;
}

void ImGuiRenderer::Shutdown() {
    if (m_dx11Initialized) {
        ImGui_ImplDX11_Shutdown();
        m_dx11Initialized = false;
    }
    if (m_win32Initialized) {
        ImGui_ImplWin32_Shutdown();
        m_win32Initialized = false;
    }
    if (m_contextCreated) {
        ImGui::DestroyContext();
        m_contextCreated = false;
    }
    if (s_instance == this) s_instance = nullptr;
    m_hwnd = nullptr;
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
    if (!m_win32Initialized) return false;
    return ImGui_ImplWin32_WndProcHandler(
        m_hwnd, msg->message, msg->wParam, msg->lParam) != 0;
}

} // namespace coco
