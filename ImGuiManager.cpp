#include "ImGuiManager.h"
#include "MemoryManager.h"

// 全局实例声明
extern MemoryManager g_memoryManager;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool ImGuiManager::Initialize(HWND hWnd) {
    hwnd = hWnd;
    
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Load Chinese font
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simsun.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    return true;
}

void ImGuiManager::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
}

void ImGuiManager::NewFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::Render() {
    ImGui::Render();
    
    const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0);
}

bool ImGuiManager::ProcessMessage(MSG* msg) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg->message, msg->wParam, msg->lParam)) {
        return true;
    }
    return false;
}

void ImGuiManager::ShowSunshineWindow(int* sunshine, int& tempSunshine) {
    // 设置窗口大小和位置
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    
    // 创建美化的窗口
    ImGui::Begin("植物大战僵尸阳光修改器", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // 标题样式
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "🌞 阳光修改器");
    ImGui::PopFont();
    ImGui::Separator();

    // 当前阳光值显示
    ImGui::Text("当前阳光值:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%d", *sunshine);
    
    ImGui::Spacing();

    // 阳光值输入
    ImGui::Text("设置阳光值:");
    ImGui::PushItemWidth(-1);
    ImGui::InputInt("", &tempSunshine, 10, 100);
    ImGui::PopItemWidth();
    
    ImGui::Spacing();

    // 应用按钮
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
    if (ImGui::Button("应用修改", ImVec2(-1, 40))) {
        *sunshine = tempSunshine;
    }
    ImGui::PopStyleColor(3);

    ImGui::Separator();

    // 进程状态
    ImGui::Text("进程状态:");
    if (g_memoryManager.IsAttached()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✅ 已连接到植物大战僵尸");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "❌ 未连接到植物大战僵尸");
        if (ImGui::Button("重新连接")) {
            g_memoryManager.AttachProcess(L"PlantsVsZombies.exe");
        }
    }

    ImGui::End();
}

void ImGuiManager::HandleResize(WPARAM wParam, LPARAM lParam) {
    if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
        CreateRenderTarget();
    }
}

bool ImGuiManager::CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void ImGuiManager::CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) {
        g_pSwapChain->Release(); g_pSwapChain = NULL;
    }
    if (g_pd3dDeviceContext) {
        g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL;
    }
    if (g_pd3dDevice) {
        g_pd3dDevice->Release(); g_pd3dDevice = NULL;
    }
}

void ImGuiManager::CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void ImGuiManager::CleanupRenderTarget() {
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL;
    }
}
