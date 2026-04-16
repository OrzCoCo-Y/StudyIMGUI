#include "ImGuiManager.h"
#include "MemoryManager.h"
#include <tchar.h>

// ==============================
// 全局管理器实例
// ==============================
ImGuiManager g_imguiManager;
MemoryManager g_memoryManager;

// ==============================
// 前置声明
// ==============================
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool PumpWindowMessages(bool& shouldQuit);
HWND CreateOverlayWindow(HINSTANCE hInstance);
bool InitializeApplication(HWND hwnd);
void RunMainLoop();
void CleanupApplication(HINSTANCE hInstance, HWND hwnd);

namespace
{
    constexpr LPCTSTR kWindowClassName = _T("ImGui Example");
    constexpr LPCTSTR kWindowTitle = _T("Plants vs Zombies Sunshine Modifier");
}

// ==============================
// 应用入口
// ==============================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    HWND hwnd = CreateOverlayWindow(hInstance);
    if (hwnd == NULL)
    {
        return 1;
    }

    if (!InitializeApplication(hwnd))
    {
        CleanupApplication(hInstance, hwnd);
        return 1;
    }

    ::ShowWindow(hwnd, nCmdShow);
    ::UpdateWindow(hwnd);
    RunMainLoop();
    CleanupApplication(hInstance, hwnd);
    return 0;
}

HWND CreateOverlayWindow(HINSTANCE hInstance)
{
    // 注册无边框覆盖层窗口类。
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, NULL, NULL, NULL, NULL, kWindowClassName, NULL };
    if (!::RegisterClassEx(&wc))
    {
        return NULL;
    }

    // 创建全屏分层弹出窗口（可交互）。
    HWND hwnd = ::CreateWindowEx(
        WS_EX_LAYERED,  // 不加 WS_EX_TRANSPARENT，确保 ImGui 可接收鼠标键盘事件。
        wc.lpszClassName,
        kWindowTitle,
        WS_POPUP,  // 无边框、无标题栏。
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );
    if (hwnd == NULL)
    {
        ::UnregisterClass(kWindowClassName, hInstance);
        return NULL;
    }

    // 将黑色区域视为透明。
    ::SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    return hwnd;
}

bool InitializeApplication(HWND hwnd)
{
    // 初始化 ImGui 图形后端与上下文。
    if (!g_imguiManager.Initialize(hwnd))
    {
        return false;
    }

    // 启动时尝试附加目标进程（失败可在 UI 中重连）。
    g_memoryManager.AttachProcess(L"PlantsVsZombies.exe");
    return true;
}

void RunMainLoop()
{
    // ==============================
    // 运行时状态
    // ==============================
    int sunshine = 0;
    int pendingSunshine = 0;  // UI 输入值（用于写入阳光）。

    // ==============================
    // 主循环：事件处理 -> 数据同步 -> UI 绘制
    // ==============================
    bool done = false;
    while (!done)
    {
        // 1) 处理系统消息（输入、尺寸变化、退出等）。
        if (!PumpWindowMessages(done) || done)
        {
            break;
        }

        // 2) 开始新的 ImGui 帧。
        g_imguiManager.NewFrame();

        // 3) 从游戏进程同步当前阳光值。
        if (g_memoryManager.IsAttached())
        {
            g_memoryManager.ReadSunshine(sunshine);
        }

        // 4) 构建功能 UI（阳光编辑、CD 开关、日志等）。
        g_imguiManager.RenderSunshineWindow(&sunshine, pendingSunshine);

        // 5) 提交渲染。
        g_imguiManager.Render();
    }
}

void CleanupApplication(HINSTANCE hInstance, HWND hwnd)
{
    // ==============================
    // 资源清理
    // ==============================
    g_imguiManager.Shutdown();
    g_memoryManager.DetachProcess();

    if (hwnd != NULL)
    {
        ::DestroyWindow(hwnd);
    }
    ::UnregisterClass(kWindowClassName, hInstance);
}

bool PumpWindowMessages(bool& shouldQuit)
{
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
        {
            shouldQuit = true;
            return false;
        }
    }
    return true;
}

// Win32 消息处理函数
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // 优先交给 ImGui 处理输入消息。
    MSG message;
    message.hwnd = hWnd;
    message.message = msg;
    message.wParam = wParam;
    message.lParam = lParam;
    message.time = 0;
    message.pt.x = 0;
    message.pt.y = 0;
    if (g_imguiManager.ProcessMessage(&message))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        // 窗口尺寸变化时重建渲染目标资源。
        g_imguiManager.HandleResize(wParam, lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // 屏蔽 ALT 激活系统菜单行为。
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
