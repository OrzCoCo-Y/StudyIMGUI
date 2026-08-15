#include "ImGuiRenderer.h"
#include "D3D11Device.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include <cstdio>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
// 将字体文件名解析为 %WINDIR%\Fonts 下的完整路径；文件不存在时返回 false
// Resolve a font file name to its full path under %WINDIR%\Fonts.
bool ResolveSystemFontPath(const char* fileName, char* outPath, size_t outSize) {
    char winDir[MAX_PATH];
    const UINT len = ::GetWindowsDirectoryA(winDir, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return false;
    snprintf(outPath, outSize, "%s\\Fonts\\%s", winDir, fileName);
    return ::GetFileAttributesA(outPath) != INVALID_FILE_ATTRIBUTES;
}
} // namespace

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
    // 覆盖层窗口不保存/加载布局，避免在运行目录生成 imgui.ini
    io.IniFilename = nullptr;

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

    // 字体文件缺失时优雅降级：基础字体加载失败即返回 nullptr（回退到 ImGui 默认字体），
    // 中文字体 / 符号字体缺失则跳过合并，UI 仍可运行。
    char basePath[MAX_PATH];
    char cjkPath[MAX_PATH];
    char symbolPath[MAX_PATH];
    const bool hasCjk    = ResolveSystemFontPath("msyh.ttc", cjkPath, MAX_PATH);
    const bool hasSymbol = ResolveSystemFontPath("seguisym.ttf", symbolPath, MAX_PATH);

    auto addFontStack = [&](const char* baseFile, float size,
                            bool mergeCjk, bool mergeSymbol) -> ImFont* {
        if (!ResolveSystemFontPath(baseFile, basePath, MAX_PATH))
            return nullptr;
        ImFont* font = io.Fonts->AddFontFromFileTTF(basePath, size, &cfg, ranges);
        if (!font) return nullptr;
        cfg.MergeMode = true;
        if (mergeCjk && hasCjk)
            io.Fonts->AddFontFromFileTTF(cjkPath, size, &cfg, ranges);
        if (mergeSymbol && hasSymbol)
            io.Fonts->AddFontFromFileTTF(symbolPath, size, &cfg, kSymbolRanges);
        cfg.MergeMode = false;
        return font;
    };

    // Larger defaults keep Chinese text crisp and readable on a standard
    // Windows desktop window instead of the former full-screen overlay.
    m_fontUi    = addFontStack("segoeui.ttf", 17.0f, true, true);
    m_fontBody  = addFontStack("segoeui.ttf", 16.0f, true, true);
    m_fontSmall = addFontStack("segoeui.ttf", 15.0f, true, true);
    m_fontTiny  = addFontStack("segoeui.ttf", 13.0f, true, true);
    m_fontMicro = addFontStack("segoeui.ttf", 11.0f, true, true);
    m_fontMono  = addFontStack("consola.ttf", 14.0f, true, false);
    m_fontTitle = addFontStack("msyhbd.ttc", 18.0f, false, true);

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
