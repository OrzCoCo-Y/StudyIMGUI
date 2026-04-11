#pragma once

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include <d3d11.h>
#include <string>

class ImGuiManager {
private:
	ID3D11Device* g_pd3dDevice = NULL;
	ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
	IDXGISwapChain* g_pSwapChain = NULL;
	ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
	HWND hwnd = NULL;
	bool showDemoWindow = true;
	bool showAnotherWindow = false;
	ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

public:
	bool Initialize(HWND hWnd);
	void Shutdown();
	void NewFrame();
	void Render();
	bool ProcessMessage(MSG* msg);
	void ShowSunshineWindow(int* sunshine, int& tempSunshine);
	void HandleResize(WPARAM wParam, LPARAM lParam);

private:
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
};