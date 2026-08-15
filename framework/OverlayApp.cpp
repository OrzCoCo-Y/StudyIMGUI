#include "OverlayApp.h"
#include "GameFeature.h"
#include "LogBuffer.h"
#include "UiTheme.h"

#include "../core/D3D11Device.h"
#include "../core/ImGuiRenderer.h"
#include "../core/Process.h"
#include "../core/Memory.h"
#include "../core/Core.h"

#include "imgui.h"
#include <shellapi.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

namespace coco {
namespace {

// 字体辅助 — 与 Feature 侧共享 ImGuiRenderer 字体图集
ImFont* UiFont() {
    return ImGuiRenderer::Instance() ? ImGuiRenderer::Instance()->FontUi() : nullptr;
}
ImFont* TinyFont() {
    return ImGuiRenderer::Instance() ? ImGuiRenderer::Instance()->FontTiny() : nullptr;
}
ImFont* SmallFont() {
    return ImGuiRenderer::Instance() ? ImGuiRenderer::Instance()->FontSmall() : nullptr;
}

void PushUiFont()    { if (ImFont* f = UiFont())    ImGui::PushFont(f, f->LegacySize); }
void PushTinyFont()  { if (ImFont* f = TinyFont())  ImGui::PushFont(f, f->LegacySize); }
void PushSmallFont() { if (ImFont* f = SmallFont()) ImGui::PushFont(f, f->LegacySize); }

} // namespace

// ==============================
// 构造 / 析构
// ==============================

OverlayApp::OverlayApp()
    : m_d3d(std::make_unique<D3D11Device>())
    , m_imgui(std::make_unique<ImGuiRenderer>())
    , m_process(std::make_unique<Process>())
    , m_memory(std::make_unique<Memory>(*m_process))
    , m_log(std::make_unique<LogBuffer>())
{
    m_log->Add(LogLevel::Info, "CoCo OverlayApp 初始化");
}

OverlayApp::~OverlayApp() {
    for (auto& f : m_features) f->OnDetach();
    m_activeFeature = nullptr;
    m_features.clear();
    m_imgui->Shutdown();
    m_d3d->Destroy();
    m_process->Detach();
}

// ==============================
// Feature 注册
// ==============================

void OverlayApp::RegisterFeature(std::unique_ptr<GameFeature> feature) {
    if (!feature) return;
    m_log->Add(LogLevel::Info, "注册 Feature: %s", feature->GetName());
    feature->OnInit();
    m_features.push_back(std::move(feature));
}

// ==============================
// 入口
// ==============================

int OverlayApp::Run(HINSTANCE hInstance, int nCmdShow) {
    // 1) 创建可移动、可缩放的桌面窗口
    if (!CreateOverlayWindow(hInstance)) {
        m_log->Add(LogLevel::Error, "创建窗口失败");
        return 1;
    }

    // 2) 初始化 D3D11 设备（必须在 ImGui 之前）
    if (!m_d3d->Create(m_hwnd)) {
        m_log->Add(LogLevel::Error, "D3D11 设备创建失败");
        DestroyOverlayWindow(hInstance);
        return 1;
    }

    // 3) 初始化 ImGui（依赖 D3D11 设备）
    if (!m_imgui->Initialize(m_hwnd, *m_d3d)) {
        m_log->Add(LogLevel::Error, "ImGui 初始化失败");
        DestroyOverlayWindow(hInstance);
        return 1;
    }

    ::ShowWindow(m_hwnd, nCmdShow);
    ::UpdateWindow(m_hwnd);
    m_log->Add(LogLevel::Info, "窗口已创建，开始主循环");

    // 4) 主循环
    bool done = false;
    while (!done) {
        // 消息泵
        if (!PumpMessages(done) || done) break;

        // 热键
        PollHotkeys();

        // 处理 Feature 的进程操作请求（附加/分离/重连）
        // Handle feature requests: attach / detach / reconnect
        HandleUiRequests();

        // 进程管理：检测目标进程退出并自动分离，然后自动附加当前 Feature
        // Auto-detach a dead target, then auto-attach the active feature's process
        if (m_activeFeature && !m_autoAttachSuppressed) {
            if (m_process->IsAttached() && !m_process->IsAlive()) {
                m_activeFeature->OnDetach();
                m_process->Detach();
                m_log->Add(LogLevel::Warning, "目标进程已退出，自动分离");
            }
            if (!m_process->IsAttached()) {
                AttachActiveFeature(false);
            }
        }

        // 每帧更新（数据同步 + 持续写入）
        m_imgui->NewFrame();
        if (m_activeFeature) {
            // 推送进程状态供 UI 展示（附加状态 / PID / 句柄）
            // Push process state for UI display
            m_activeFeature->OnFrameProcessState(
                m_process->IsAttached(), m_process->ProcessId(), m_process->Handle());
            if (m_process->IsAttached()) {
                m_activeFeature->OnUpdate(*m_memory);
            }
        }

        // 绘制 UI（如果窗口可见）
        if (m_windowVisible) {
            DrawTitleBar();
            for (auto& f : m_features) f->OnRenderUI();
        }

        // 渲染
        const float clear[4] = { 0, 0, 0, 0 };
        m_d3d->BeginFrame(clear);
        if (m_windowVisible) m_imgui->Render();
        m_d3d->EndFrame(true);
    }

    m_log->Add(LogLevel::Info, "主循环退出");
    m_imgui->Shutdown();
    m_process->Detach();
    DestroyOverlayWindow(hInstance);
    return 0;
}

// ==============================
// UI：自定义标题栏（替代系统标题栏 + 原 ImGui 主菜单栏）
// ==============================

void OverlayApp::DrawTitleBar() {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float barH = kTitleBarHeight;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(display.x, barH));

    PushUiFont();
    const float padY = (barH - ImGui::GetTextLineHeight()) * 0.5f;  // 内容垂直居中
    ImGui::PopFont();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, kSurfaceBg);
    ImGui::PushStyleColor(ImGuiCol_Text, kText);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, kTextDim);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kHoverBg);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kActiveBg);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, kSurfaceBg);
    ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
    ImGui::PushStyleColor(ImGuiCol_Header, kAccentBg);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kHoverAccent);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, kActiveAccent);
    ImGui::PushStyleColor(ImGuiCol_Separator, kBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, padY));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (!ImGui::Begin("##CoCoTitleBar", nullptr, flags)) {
        ImGui::End();
        ImGui::PopStyleVar(6);
        ImGui::PopStyleColor(12);
        return;
    }

    // --- 品牌：◈ CoCo v2.0 ---
    PushUiFont();
    ImGui::TextColored(kAccent, "◈");
    ImGui::SameLine(0, 2.0f);
    ImGui::TextColored(kTextBright, "CoCo");
    ImGui::PopFont();
    ImGui::SameLine(0, 6.0f);
    PushTinyFont();
    ImGui::TextColored(kTextDim, "v2.0");
    ImGui::PopFont();

    // --- 功能菜单（保留原主菜单栏能力：切换 Feature / 退出） ---
    ImGui::SameLine(0, 16.0f);
    PushSmallFont();
    if (ImGui::Button("功能 ▾")) ImGui::OpenPopup("##CoCoFeatureMenu");
    ImGui::PopFont();
    const ImVec2 menuMin = ImGui::GetItemRectMin();
    const ImVec2 menuMax = ImGui::GetItemRectMax();

    if (ImGui::BeginPopup("##CoCoFeatureMenu")) {
        PushSmallFont();
        for (auto& f : m_features) {
            bool isActive = (f.get() == m_activeFeature);
            if (ImGui::MenuItem(f->GetName(), nullptr, &isActive)) {
                if (isActive) {
                    if (m_activeFeature) m_activeFeature->OnDetach();
                    m_process->Detach();
                    m_autoAttachSuppressed = false;
                    m_activeFeature = f.get();
                    m_log->Add(LogLevel::Info, "切换到: %s",
                               m_activeFeature->GetName());
                } else {
                    if (m_activeFeature) m_activeFeature->OnDetach();
                    m_process->Detach();
                    m_autoAttachSuppressed = false;
                    m_activeFeature = nullptr;
                }
            }
        }
        // 保留 Feature 自定义菜单扩展点（原主菜单栏中的 OnRenderMenuBar）
        for (auto& f : m_features) f->OnRenderMenuBar();
        ImGui::Separator();
        if (ImGui::MenuItem("退出")) { ::PostQuitMessage(0); }
        ImGui::PopFont();
        ImGui::EndPopup();
    }

    // --- 右侧窗口控制按钮（最小化 / 最大化 / 关闭） ---
    const float ctlTotal = kWinCtlCount * kWinCtlWidth +
                           (kWinCtlCount - 1) * kWinCtlGap;
    const float ctlStartX = display.x - ctlTotal;
    DrawWindowControls(barH, ctlStartX);

    // --- 拖动区域：标题栏空白处（品牌左侧 + 菜单按钮与窗口控制之间），按钮区域除外 ---
    const bool overDrag =
        (menuMin.x - 6.0f > 0.0f &&
         ImGui::IsMouseHoveringRect(ImVec2(0, 0), ImVec2(menuMin.x - 6.0f, barH))) ||
        (ctlStartX - 6.0f > menuMax.x + 6.0f &&
         ImGui::IsMouseHoveringRect(ImVec2(menuMax.x + 6.0f, 0),
                                    ImVec2(ctlStartX - 6.0f, barH)));
    if (overDrag && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        ToggleMaximize();
    if (overDrag && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 5.0f))
        StartTitleBarDrag();

    // 标题栏底部 1px 分隔线
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine(ImVec2(0, barH - 1), ImVec2(display.x, barH - 1),
                ToU32(kBorder), 1.0f);

    ImGui::End();
    ImGui::PopStyleVar(6);
    ImGui::PopStyleColor(12);
}

void OverlayApp::DrawWindowControls(float stripHeight, float startX) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 winPos = ImGui::GetWindowPos();
    const float w = kWinCtlWidth, gap = kWinCtlGap;

    for (int i = 0; i < kWinCtlCount; ++i) {
        // 注意：不能命名为 min/max（windows.h 宏）
        const ImVec2 btnMin(startX + i * (w + gap), winPos.y);
        const ImVec2 btnMax(btnMin.x + w, btnMin.y + stripHeight);
        const bool hovered = ImGui::IsMouseHoveringRect(btnMin, btnMax);
        const bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        const bool isClose = (i == 2);

        if (hovered)
            dl->AddRectFilled(btnMin, btnMax, ToU32(isClose ? kRed : kHoverBg), 3.0f);

        const ImU32 iconCol = hovered
            ? ToU32(isClose ? ImVec4(1, 1, 1, 1) : kTextBright)
            : ToU32(kTextDim);
        const ImVec2 c((btnMin.x + btnMax.x) * 0.5f, (btnMin.y + btnMax.y) * 0.5f);

        switch (i) {
        case 0:  // 最小化 —
            dl->AddLine(ImVec2(c.x - 5.5f, c.y), ImVec2(c.x + 5.5f, c.y),
                        iconCol, 1.8f);
            break;
        case 1:  // 最大化 / 还原
            if (::IsZoomed(m_hwnd)) {
                dl->AddRect(ImVec2(c.x - 8.0f, c.y - 8.0f),
                            ImVec2(c.x + 3.0f, c.y + 3.0f), iconCol, 1.0f, 0, 1.6f);
                dl->AddRect(ImVec2(c.x - 3.0f, c.y - 3.0f),
                            ImVec2(c.x + 8.0f, c.y + 8.0f), iconCol, 1.0f, 0, 1.6f);
            } else {
                dl->AddRect(ImVec2(c.x - 5.5f, c.y - 5.5f),
                            ImVec2(c.x + 5.5f, c.y + 5.5f), iconCol, 1.0f, 0, 1.6f);
            }
            break;
        case 2:  // 关闭 ✕
            dl->AddLine(ImVec2(c.x - 5.0f, c.y - 5.0f), ImVec2(c.x + 5.0f, c.y + 5.0f),
                        iconCol, 1.8f);
            dl->AddLine(ImVec2(c.x - 5.0f, c.y + 5.0f), ImVec2(c.x + 5.0f, c.y - 5.0f),
                        iconCol, 1.8f);
            break;
        }

        if (clicked) {
            switch (i) {
            case 0: ::ShowWindow(m_hwnd, SW_MINIMIZE); break;
            case 1: ToggleMaximize(); break;
            case 2: ::PostQuitMessage(0); break;
            }
        }
    }
}

void OverlayApp::ToggleMaximize() {
    if (::IsZoomed(m_hwnd))
        ::ShowWindow(m_hwnd, SW_RESTORE);
    else
        ::ShowWindow(m_hwnd, SW_MAXIMIZE);
}

void OverlayApp::StartTitleBarDrag() {
    // 借系统标题栏拖动循环：拖动移动、从最大化拖下还原跟随鼠标，与原生行为一致
    ::ReleaseCapture();
    ::SendMessage(m_hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

    // 拖动结束后按释放位置执行贴边吸附：顶边最大化 / 左右半屏
    POINT pt;
    ::GetCursorPos(&pt);
    HMONITOR mon = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{ sizeof(mi) };
    if (!::GetMonitorInfo(mon, &mi)) return;
    const RECT& wa = mi.rcWork;
    const int margin = 2;
    if (pt.y <= wa.top + margin) {
        if (!::IsZoomed(m_hwnd)) ::ShowWindow(m_hwnd, SW_MAXIMIZE);
        return;
    }
    const int halfW = (wa.right - wa.left) / 2;
    if (pt.x <= wa.left + margin) {
        ::SetWindowPos(m_hwnd, nullptr, wa.left, wa.top, halfW,
                       wa.bottom - wa.top, SWP_NOZORDER | SWP_NOACTIVATE);
    } else if (pt.x >= wa.right - margin) {
        ::SetWindowPos(m_hwnd, nullptr, wa.right - halfW, wa.top, halfW,
                       wa.bottom - wa.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

// ==============================
// Feature 请求处理
// ==============================

void OverlayApp::HandleUiRequests() {
    for (auto& feature : m_features) {
        const GameFeature::UiRequest request = feature->ConsumeUiRequest();
        if (request == GameFeature::UiRequest::None) continue;

        // 若无活动 Feature，自动激活发起请求的 Feature
        // Auto-activate the requesting feature when nothing is active
        if (!m_activeFeature) m_activeFeature = feature.get();

        if (feature.get() != m_activeFeature) {
            m_log->Add(LogLevel::Warning, "请先在菜单中切换到 %s", feature->GetName());
            continue;
        }

        switch (request) {
            case GameFeature::UiRequest::Attach:
                if (!m_process->IsAttached()) {
                    m_autoAttachSuppressed = false;
                    AttachActiveFeature(true);
                }
                break;
            case GameFeature::UiRequest::Detach:
                if (m_process->IsAttached()) {
                    m_activeFeature->OnDetach();
                    m_process->Detach();
                    m_autoAttachSuppressed = true;
                    m_log->Add(LogLevel::Info, "已分离 %s", m_activeFeature->GetName());
                }
                break;
            case GameFeature::UiRequest::Reconnect:
                if (m_process->IsAttached()) m_activeFeature->OnDetach();
                m_process->Detach();
                m_autoAttachSuppressed = false;
                AttachActiveFeature(true);
                break;
            default:
                break;
        }
    }
}

// ==============================
// 附加当前 Feature 目标进程
// ==============================

bool OverlayApp::AttachActiveFeature(bool notifyFailure) {
    if (!m_activeFeature) return false;
    if (!m_process->Attach(m_activeFeature->GetProcessName())) {
        if (notifyFailure)
            m_log->Add(LogLevel::Warning, "附加失败: %s", m_activeFeature->GetProcessName());
        return false;
    }
    m_activeFeature->OnAttach(*m_memory);
    m_log->Add(LogLevel::Info, "已附加到 %s", m_activeFeature->GetProcessName());
    return true;
}

// ==============================
// 窗口
// ==============================

bool OverlayApp::CreateOverlayWindow(HINSTANCE hInstance) {
    WNDCLASSEX wc{};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_CLASSDC;
    wc.lpfnWndProc   = StaticWndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"CoCoOverlay";

    if (!::RegisterClassEx(&wc)) return false;

    // 无边框窗口：去掉系统标题栏（WS_CAPTION），保留可缩放边（WS_THICKFRAME）
    // 与最小化/最大化能力；客户区即窗口本身，860x560 与 UI 布局完全一致。
    // 标题栏由 ImGui 自绘（DrawTitleBar），缩小/最大化/关闭按钮与整体 UI 风格统一。
    RECT rc{ 0, 0, 860, 560 };
    const DWORD winStyle =
        WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
    const int winW = rc.right - rc.left;
    const int winH = rc.bottom - rc.top;
    const int screenW = ::GetSystemMetrics(SM_CXSCREEN);
    const int screenH = ::GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - winW) / 2;
    int y = (screenH - winH) / 2 - 20;
    if (x < 0) x = 0;
    if (y < 40) y = 40;

    m_hwnd = ::CreateWindowEx(
        0, wc.lpszClassName, L"CoCo - Game Modifier Tool",
        winStyle, x, y, winW, winH,
        nullptr, nullptr, hInstance, this);

    if (!m_hwnd) {
        ::UnregisterClass(wc.lpszClassName, hInstance);
        return false;
    }

    // Windows 11 圆角窗口（DWMWA_WINDOW_CORNER_PREFERENCE = 33, DWMWCP_ROUND = 2）。
    // 旧系统上该属性不可用，调用失败时保持直角，不影响功能。
    const DWORD cornerPref = 2;
    ::DwmSetWindowAttribute(m_hwnd, 33, &cornerPref, sizeof(cornerPref));

    // 无边框窗口默认没有投影，向下扩展 1px 框架让 DWM 恢复窗口阴影
    MARGINS shadowMargins{ 0, 0, 1, 0 };
    ::DwmExtendFrameIntoClientArea(m_hwnd, &shadowMargins);
    return true;
}

void OverlayApp::DestroyOverlayWindow(HINSTANCE hInstance) {
    if (m_hwnd) { ::DestroyWindow(m_hwnd); m_hwnd = nullptr; }
    ::UnregisterClass(L"CoCoOverlay", hInstance);
}

bool OverlayApp::PumpMessages(bool& shouldQuit) {
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT) { shouldQuit = true; return false; }
    }
    return true;
}

// ==============================
// 热键
// ==============================

void OverlayApp::PollHotkeys() {
    // 输入框获得焦点时禁用全局热键，避免 HOME/END 移动光标被误判为显隐切换
    if (ImGui::GetIO().WantTextInput) {
        m_homeWasDown = false;
        m_endWasDown  = false;
        return;
    }

    bool homeDown = (::GetAsyncKeyState(VK_HOME) & 0x8000) != 0;
    bool endDown  = (::GetAsyncKeyState(VK_END)  & 0x8000) != 0;

    if (homeDown && !m_homeWasDown) {
        m_windowVisible = true;
        ::ShowWindow(m_hwnd, SW_SHOW);
    }
    if (endDown && !m_endWasDown) {
        m_windowVisible = false;
        ::ShowWindow(m_hwnd, SW_HIDE);
    }
    m_homeWasDown = homeDown;
    m_endWasDown  = endDown;
}

// ==============================
// WndProc
// ==============================

LRESULT WINAPI OverlayApp::StaticWndProc(HWND hWnd, UINT msg,
                                          WPARAM wParam, LPARAM lParam) {
    OverlayApp* app = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = static_cast<OverlayApp*>(cs->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA,
                           reinterpret_cast<LONG_PTR>(app));
    } else {
        app = reinterpret_cast<OverlayApp*>(
            ::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    if (app) return app->WndProc(hWnd, msg, wParam, lParam);
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT OverlayApp::WndProc(HWND hWnd, UINT msg,
                             WPARAM wParam, LPARAM lParam) {
    MSG m{};
    m.hwnd = hWnd; m.message = msg; m.wParam = wParam; m.lParam = lParam;
    if (m_imgui->ProcessMessage(&m)) return true;

    switch (msg) {
        case WM_NCCALCSIZE:
            // 无边框窗口：客户区铺满整个窗口（wParam=TRUE 表示要求计算客户区），
            // 彻底消除系统绘制的白色 NC 边框；边缘缩放由 WM_NCHITTEST 手动提供。
            if (wParam) return 0;
            break;
        case WM_NCHITTEST: {
            // 无边框窗口：手动提供 8px 边缘/角落的缩放命中区，其余交给 DefWindowProc
            // （标题栏拖动由 ImGui 侧 ReleaseCapture + WM_NCLBUTTONDOWN(HTCAPTION) 处理）
            POINT pt{ static_cast<short>(LOWORD(lParam)),
                      static_cast<short>(HIWORD(lParam)) };
            RECT rc{};
            ::GetWindowRect(hWnd, &rc);
            const int border = 8;
            const bool left   = pt.x <  rc.left   + border;
            const bool right  = pt.x >= rc.right  - border;
            const bool top    = pt.y <  rc.top    + border;
            const bool bottom = pt.y >= rc.bottom - border;
            if (top && left)     return HTTOPLEFT;
            if (top && right)    return HTTOPRIGHT;
            if (bottom && left)  return HTBOTTOMLEFT;
            if (bottom && right) return HTBOTTOMRIGHT;
            if (left)            return HTLEFT;
            if (right)           return HTRIGHT;
            if (top)             return HTTOP;
            if (bottom)          return HTBOTTOM;
            break;
        }
        case WM_GETMINMAXINFO: {
            // 无边框窗口：最大化 = 显示器工作区；尺寸约束与 UI 一致（客户区即窗口本身）
            MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
            RECT wa{};
            if (HMONITOR mon = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST)) {
                MONITORINFO mi{ sizeof(mi) };
                if (::GetMonitorInfo(mon, &mi)) wa = mi.rcWork;
            }
            mmi->ptMaxPosition.x = wa.left;
            mmi->ptMaxPosition.y = wa.top;
            mmi->ptMaxSize.x = wa.right - wa.left;
            mmi->ptMaxSize.y = wa.bottom - wa.top;
            mmi->ptMinTrackSize.x = 700;
            mmi->ptMinTrackSize.y = 460;
            mmi->ptMaxTrackSize.x = 1200;
            mmi->ptMaxTrackSize.y = 900;
            return 0;
        }
        case WM_SIZE:
            m_d3d->Resize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) == SC_KEYMENU) return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

} // namespace coco