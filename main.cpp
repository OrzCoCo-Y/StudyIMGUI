#include "ImGuiManager.h"
#include "MemoryManager.h"
#include <tchar.h>

// Global instances
ImGuiManager g_imguiManager;
MemoryManager g_memoryManager;

// Forward declarations
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Create application window with no border and transparent background
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    
    // Create borderless window
    HWND hwnd = ::CreateWindowEx(
        WS_EX_LAYERED,  // Extended styles for layered window (no WS_EX_TRANSPARENT to allow mouse events)
        wc.lpszClassName,
        _T("Plants vs Zombies Sunshine Modifier"),
        WS_POPUP,  // No border or title bar
        0, 0,  // Full screen
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );
    
    // Set window transparency
    ::SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // Initialize ImGui
    if (!g_imguiManager.Initialize(hwnd))
    {
        ::UnregisterClass(wc.lpszClassName, hInstance);
        return 1;
    }

    // Attach to Plants vs Zombies process
    bool attached = g_memoryManager.AttachProcess(L"PlantsVsZombies.exe");

    // Show the window
    ::ShowWindow(hwnd, nCmdShow);
    ::UpdateWindow(hwnd);

    // Our state
    int sunshine = 0;
    bool showDemoWindow = false;
    // 在 main 函数的 while 循环外部，声明临时变量

    int tempSunshine = 0;

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // 开始新的 ImGui 帧
        g_imguiManager.NewFrame();


        // 从游戏中读取阳光值到 sunshine 变量
        if (g_memoryManager.IsAttached()) {
            g_memoryManager.ReadSunshine(sunshine);
        }

        // 修改这里：将临时变量 tempSunshine 也传递进去
        g_imguiManager.ShowSunshineWindow(&sunshine, tempSunshine);


        // Rendering
        g_imguiManager.Render();
    }

    // Cleanup
    g_imguiManager.Shutdown();
    g_memoryManager.DetachProcess();
    
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, hInstance);

    return 0;
}

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Process ImGui messages
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
        g_imguiManager.HandleResize(wParam, lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
