#pragma once

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include <d3d11.h>
#include <string>
#include <vector>

class ImGuiManager {
private:
	ID3D11Device* m_pd3dDevice = nullptr;
	ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;
	HWND m_hwnd = nullptr;
	bool m_showLogWindow = false;
	bool m_cdSlot1Enabled = false;
	bool m_cdSlot2Enabled = false;
	bool m_cdSlot3Enabled = false;
	bool m_autoCollectSunshine = false;
	std::vector<std::string> m_logMessages;

public:
	bool Initialize(HWND hWnd);
	void Shutdown();
	void NewFrame();
	void Render();
	bool ProcessMessage(MSG* msg);
	void RenderSunshineWindow(int* sunshine, int& pendingSunshine);
	void HandleResize(WPARAM wParam, LPARAM lParam);
	void AddLog(const std::string& message);
	void ShowLogWindow();

private:
	void RenderSunshineControls(const int& sunshine, int& pendingSunshine);
	void RenderFeatureToggles();
	void RenderProcessStatus();
	void ApplyContinuousFeatures();

	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
};