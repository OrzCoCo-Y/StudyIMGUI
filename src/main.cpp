// ==============================
// 应用入口：植物大战僵尸阳光修改器
// 功能：使用 Dear ImGui + DirectX 11 创建一个全屏透明覆盖层，
// 附加到 PlantsVsZombies.exe 进程，提供阳光修改、CD 格禁用、
// 自动采集阳光等功能。Home 键显示界面，End 键隐藏。
// ==============================

#include "core/ImGuiManager.h"
#include "core/MemoryManager.h"
#include <tchar.h>

// 全局管理器实例
ImGuiManager g_imguiManager;
MemoryManager g_memoryManager;

// 前置声明
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool PumpWindowMessages(bool& shouldQuit);
HWND CreateOverlayWindow(HINSTANCE hInstance);
bool InitializeApplication(HWND hwnd);
void RunMainLoop(HWND hwnd);
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
    RunMainLoop(hwnd);
    CleanupApplication(hInstance, hwnd);
    return 0;
}

// ==============================
// 创建全屏透明覆盖层窗口
// ==============================
HWND CreateOverlayWindow(HINSTANCE hInstance)
{
    // 注册无边框覆盖层窗口类
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, NULL, NULL, NULL, NULL, kWindowClassName, NULL };
    if (!::RegisterClassEx(&wc))
    {
        return NULL;
    }

    // 创建全屏分层弹出窗口（可交互）
    // WS_EX_LAYERED 启用分层窗口，配合 LWA_COLORKEY 将黑色区域设为透明
    // 注意不加 WS_EX_TRANSPARENT，确保 ImGui 可正常接收鼠标键盘事件
    HWND hwnd = ::CreateWindowEx(
        WS_EX_LAYERED,
        wc.lpszClassName,
        kWindowTitle,
        WS_POPUP,
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

    // 将黑色 (0, 0, 0) 区域视为透明，使覆盖层下方的游戏画面可见
    ::SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    return hwnd;
}

// ==============================
// 初始化 ImGui 渲染后段并附加游戏进程
// ==============================
bool InitializeApplication(HWND hwnd)
{
    // 初始化 ImGui 图形后端与上下文
    if (!g_imguiManager.Initialize(hwnd))
    {
        return false;
    }

    // 启动时尝试附加目标进程（失败可在 UI 中手动重连）
    g_memoryManager.AttachProcess(L"PlantsVsZombies.exe");
    return true;
}

// ==============================
// 主消息循环
// ==============================
void RunMainLoop(HWND hwnd)
{
    // 运行时状态
    int sunshine = 0;
    int pendingSunshine = 0;  // UI 输入值（用于写入阳光）
    bool homeKeyPressed = false;
    bool endKeyPressed = false;

    // 主循环：事件处理 -> 数据同步 -> UI 绘制
    bool done = false;
    while (!done)
    {
        // 1) 处理系统消息（输入、尺寸变化、退出等）
        if (!PumpWindowMessages(done) || done)
        {
            break;
        }

        // 2) 热键处理：Home 显示窗口，End 隐藏窗口
        // 使用按下沿触发，避免长按反复执行
        bool isHomeDown = (::GetAsyncKeyState(VK_HOME) & 0x8000) != 0;
        bool isEndDown = (::GetAsyncKeyState(VK_END) & 0x8000) != 0;
        if (isHomeDown && !homeKeyPressed)
        {
            ::ShowWindow(hwnd, SW_SHOW);
        }
        if (isEndDown && !endKeyPressed)
        {
            ::ShowWindow(hwnd, SW_HIDE);
        }
        homeKeyPressed = isHomeDown;
        endKeyPressed = isEndDown;

        // 3) 开始新的 ImGui 帧
        g_imguiManager.NewFrame();

        // 4) 从游戏进程同步当前阳光值
        if (g_memoryManager.IsAttached())
        {
            g_memoryManager.ReadSunshine(sunshine);
        }

        // 5) 构建功能 UI（阳光编辑、CD 开关、日志等）
        g_imguiManager.RenderSunshineWindow(&sunshine, pendingSunshine);

        // 6) 提交渲染
        g_imguiManager.Render();
    }
}

// ==============================
// 清理资源
// ==============================
void CleanupApplication(HINSTANCE hInstance, HWND hwnd)
{
    g_imguiManager.Shutdown();
    g_memoryManager.DetachProcess();

    if (hwnd != NULL)
    {
        ::DestroyWindow(hwnd);
    }
    ::UnregisterClass(kWindowClassName, hInstance);
}

// ==============================
// 窗口消息泵
// ==============================
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

// ==============================
// Win32 窗口过程
// ==============================
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // 优先交给 ImGui 后端处理输入消息（鼠标、键盘、滚轮等）
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
        // 窗口尺寸变化时重建 D3D11 渲染目标资源
        g_imguiManager.HandleResize(wParam, lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;  // 屏蔽 Alt 激活系统菜单的行为
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
