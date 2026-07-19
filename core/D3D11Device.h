#pragma once

#include <d3d11.h>
#include <windows.h>

namespace coco {

// ==============================
// D3D11Device — DirectX 11 设备管理
// ==============================
// 职责：创建/销毁 D3D11 设备、交换链、渲染目标视图。
// 不依赖 ImGui，可独立使用或替换。
class D3D11Device {
public:
    D3D11Device() = default;
    ~D3D11Device();

    // 不可拷贝
    D3D11Device(const D3D11Device&) = delete;
    D3D11Device& operator=(const D3D11Device&) = delete;

    // 创建 D3D11 设备与交换链，关联到指定窗口
    bool Create(HWND hWnd);

    // 释放所有 D3D11 资源
    void Destroy();

    // 响应窗口尺寸变化，重建渲染目标
    bool Resize(UINT width, UINT height);

    // Clear render target to given color, then set as OM target
    void BeginFrame(const float clearColor[4]);

    // Present the swap chain
    void EndFrame(bool vsync = true);

    // --- 访问器 ---
    ID3D11Device*        Device()       const { return m_device; }
    ID3D11DeviceContext* Context()      const { return m_context; }
    IDXGISwapChain*      SwapChain()    const { return m_swapChain; }
    HWND                 WindowHandle() const { return m_hwnd; }

private:
    void CreateRenderTarget();
    void DestroyRenderTarget();

    HWND                m_hwnd           = nullptr;
    ID3D11Device*       m_device         = nullptr;
    ID3D11DeviceContext* m_context       = nullptr;
    IDXGISwapChain*     m_swapChain      = nullptr;
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
};

} // namespace coco
