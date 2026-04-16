#include "ImGuiManager.h"
#include "MemoryManager.h"
#include <time.h>

// 全局实例声明
extern MemoryManager g_memoryManager;

// 前置声明：来自 imgui_impl_win32.cpp 的消息处理函数
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool ImGuiManager::Initialize(HWND hWnd) {
    m_hwnd = hWnd;
    
    if (!CreateDeviceD3D(m_hwnd)) {
        CleanupDeviceD3D();
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // 加载中文字体，避免界面中文出现乱码
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simsun.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dDeviceContext);

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
    m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
    m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    m_pSwapChain->Present(1, 0);
}

bool ImGuiManager::ProcessMessage(MSG* msg) {
    if (ImGui_ImplWin32_WndProcHandler(m_hwnd, msg->message, msg->wParam, msg->lParam)) {
        return true;
    }
    return false;
}

void ImGuiManager::AddLog(const std::string& message) {
    // 添加时间戳
    time_t now = time(0);
    struct tm localTime;
    localtime_s(&localTime, &now);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &localTime);
    
    std::string logEntry = std::string("[") + timeStr + "] " + message;
    m_logMessages.push_back(logEntry);
    
    // 限制日志数量，防止内存占用过大
    if (m_logMessages.size() > 100) {
        m_logMessages.erase(m_logMessages.begin());
    }
}

void ImGuiManager::RenderLogPanel() {
    // 清空日志按钮
    if (ImGui::Button("清空日志")) {
        m_logMessages.clear();
        AddLog("日志已清空");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("保存日志")) {
        // 简单的日志保存功能
        FILE* file;
        errno_t err = fopen_s(&file, "log.txt", "w");
        if (err == 0 && file) {
            for (size_t i = 0; i < m_logMessages.size(); i++) {
                fprintf(file, "%s\n", m_logMessages[i].c_str());
            }
            fclose(file);
            AddLog("日志已保存到 log.txt");
        } else {
            AddLog("保存日志失败");
        }
    }
    
    ImGui::Separator();
    
    // 日志显示区域采用自适应高度，并设置最小可视高度，避免窗口较小时日志被完全挤压。
    float availableHeight = ImGui::GetContentRegionAvail().y;
    float logPanelHeight = availableHeight > 140.0f ? availableHeight : 140.0f;
    ImGui::BeginChild("LogScroll", ImVec2(0, logPanelHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (size_t i = 0; i < m_logMessages.size(); i++) {
        ImGui::TextUnformatted(m_logMessages[i].c_str());
    }
    
    // 自动滚动到底部
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

void ImGuiManager::RenderSunshineWindow(int* sunshine, int& pendingSunshine) {
    // 设置窗口大小和位置
    ImGui::SetNextWindowSize(ImVec2(420, 430), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    
    // 创建主功能窗口
    static bool isOpen = true;
    // 行业惯例：主工具窗口允许用户按需拉伸，便于容纳日志等调试信息。
    ImGui::Begin("植物大战僵尸阳光修改器", &isOpen);
    
    // 点击窗口关闭按钮后退出程序
    if (!isOpen) {
        ::PostQuitMessage(0);
    }

    // 标题样式
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "阳光修改器");
    ImGui::PopFont();
    ImGui::Separator();

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("绘制")) {
            ImGui::Text("当前阳光值:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%d", *sunshine);
            ImGui::Separator();
            RenderProcessStatus();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("修改")) {
            RenderSunshineControls(*sunshine, pendingSunshine);
            ImGui::Separator();
            RenderFeatureToggles();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // 持续功能按当前开关状态每帧生效，不受标签页切换影响。
    ApplyContinuousFeatures();

    // 大项目常见做法：业务操作与开发日志分区，日志作为可折叠调试面板默认收起。
    ImGui::Separator();
    if (ImGui::CollapsingHeader("开发者日志"))
    {
        ImGui::Text("日志条数: %d", static_cast<int>(m_logMessages.size()));
        RenderLogPanel();
    }

    ImGui::End();
}

void ImGuiManager::RenderSunshineControls(const int& sunshine, int& pendingSunshine) {
    // 阳光值输入
    ImGui::Text("设置阳光值:");
    ImGui::PushItemWidth(-1);
    ImGui::InputInt("##sunshine", &pendingSunshine, 10, 100);
    ImGui::PopItemWidth();

    ImGui::Spacing();

    // 应用按钮
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
    if (ImGui::Button("应用修改", ImVec2(-1, 40))) {
        g_memoryManager.WriteSunshine(pendingSunshine);
        AddLog("阳光值已修改为: " + std::to_string(pendingSunshine));
    }
    ImGui::PopStyleColor(3);
}

void ImGuiManager::RenderFeatureToggles() {
    // CD 格功能开关
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "CD格功能开关");
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));

    if (ImGui::Checkbox("1格CD (无CD)", &m_cdSlot1Enabled)) {
        AddLog(std::string("1格CD 已") + (m_cdSlot1Enabled ? "启用" : "禁用"));
    }
    if (ImGui::Checkbox("2格CD (无CD)", &m_cdSlot2Enabled)) {
        AddLog(std::string("2格CD 已") + (m_cdSlot2Enabled ? "启用" : "禁用"));
    }
    if (ImGui::Checkbox("3格CD (无CD)", &m_cdSlot3Enabled)) {
        AddLog(std::string("3格CD 已") + (m_cdSlot3Enabled ? "启用" : "禁用"));
    }

    ImGui::Separator();

    // 自动采集阳光功能
    if (ImGui::Checkbox("自动采集阳光", &m_autoCollectSunshine)) {
        AddLog(std::string("自动采集阳光 已") + (m_autoCollectSunshine ? "启用" : "禁用"));
    }

    ImGui::PopStyleColor(2);
}

void ImGuiManager::ApplyContinuousFeatures() {
    // 持续应用功能状态（类似 CE 锁定）
    if (!g_memoryManager.IsAttached()) {
        return;
    }

    if (m_cdSlot1Enabled) {
        g_memoryManager.WriteCDSlot(1, true);
    }
    if (m_cdSlot2Enabled) {
        g_memoryManager.WriteCDSlot(2, true);
    }
    if (m_cdSlot3Enabled) {
        g_memoryManager.WriteCDSlot(3, true);
    }
    if (m_autoCollectSunshine) {
        g_memoryManager.CollectSunshine();
    }
}

void ImGuiManager::RenderProcessStatus() {
    // 进程状态
    ImGui::Text("进程状态:");
    if (g_memoryManager.IsAttached()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "已连接到植物大战僵尸");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "未连接到植物大战僵尸");
        if (ImGui::Button("重新连接")) {
            bool attached = g_memoryManager.AttachProcess(L"PlantsVsZombies.exe");
            AddLog(attached ? "已连接到植物大战僵尸" : "连接失败");
        }
    }
}

void ImGuiManager::HandleResize(WPARAM wParam, LPARAM lParam) {
    if (m_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
        CleanupRenderTarget();
        m_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
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
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void ImGuiManager::CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (m_pSwapChain) {
        m_pSwapChain->Release(); m_pSwapChain = nullptr;
    }
    if (m_pd3dDeviceContext) {
        m_pd3dDeviceContext->Release(); m_pd3dDeviceContext = nullptr;
    }
    if (m_pd3dDevice) {
        m_pd3dDevice->Release(); m_pd3dDevice = nullptr;
    }
}

void ImGuiManager::CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
    pBackBuffer->Release();
}

void ImGuiManager::CleanupRenderTarget() {
    if (m_mainRenderTargetView) {
        m_mainRenderTargetView->Release(); m_mainRenderTargetView = nullptr;
    }
}
