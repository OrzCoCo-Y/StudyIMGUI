#include "OverlayApp.h"
#include "GameFeature.h"
#include "LogBuffer.h"

#include "../core/D3D11Device.h"
#include "../core/ImGuiRenderer.h"
#include "../core/Process.h"
#include "../core/Memory.h"
#include "../core/Core.h"

#include "imgui.h"
#include <shellapi.h>

namespace coco {

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

        // 进程管理：自动附加当前 Feature 的目标进程
        // Auto-attach the active feature's target process
        if (m_activeFeature && !m_process->IsAttached() && !m_autoAttachSuppressed) {
            AttachActiveFeature(false);
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
            DrawMenuBar();
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
// UI：菜单栏
// ==============================

void OverlayApp::DrawMenuBar() {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("CoCo")) {
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
        ImGui::Separator();
        if (ImGui::MenuItem("退出")) { ::PostQuitMessage(0); }
        ImGui::EndMenu();
    }

    for (auto& f : m_features) f->OnRenderMenuBar();

    // 状态栏
    if (m_activeFeature) {
        char buf[64];
        snprintf(buf, sizeof(buf), "当前: %s", m_activeFeature->GetName());
        ImGui::SameLine(ImGui::GetWindowWidth() - 220);
        if (m_process->IsAttached())
            ImGui::TextColored(ImVec4(0,1,0,1), "%s [已连接]", buf);
        else
            ImGui::TextColored(ImVec4(1,0,0,1), "%s [未连接]", buf);
    }

    ImGui::EndMainMenuBar();
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

    // 普通桌面窗口：带标题栏，可拖动 / 调整大小。
    // 客户区初始 860x560，与 UI 布局一致 —— 窗口框即 UI 框。
    RECT rc{ 0, 0, 860, 560 };
    const DWORD winStyle = WS_OVERLAPPEDWINDOW;
    ::AdjustWindowRectEx(&rc, winStyle, FALSE, 0);
    const int winW = rc.right - rc.left;
    const int winH = rc.bottom - rc.top;
    const int screenW = ::GetSystemMetrics(SM_CXSCREEN);
    const int screenH = ::GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - winW) / 2;
    int y = (screenH - winH) / 2 - 20;
    if (x < 0) x = 0;
    if (y < 40) y = 40;

    m_hwnd = ::CreateWindowEx(
        0, wc.lpszClassName, L"CoCo - PvZ Menu Framework",
        winStyle, x, y, winW, winH,
        nullptr, nullptr, hInstance, this);

    if (!m_hwnd) {
        ::UnregisterClass(wc.lpszClassName, hInstance);
        return false;
    }
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
        case WM_GETMINMAXINFO: {
            // 与 UI 尺寸约束一致：客户区最小 700x460，最大 1200x900
            MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
            RECT rc{ 0, 0, 700, 460 };
            ::AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);
            mmi->ptMinTrackSize.x = rc.right - rc.left;
            mmi->ptMinTrackSize.y = rc.bottom - rc.top;
            rc = RECT{ 0, 0, 1200, 900 };
            ::AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);
            mmi->ptMaxTrackSize.x = rc.right - rc.left;
            mmi->ptMaxTrackSize.y = rc.bottom - rc.top;
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