#include "D3D11Device.h"

namespace coco {

D3D11Device::~D3D11Device() { Destroy(); }

bool D3D11Device::Create(HWND hWnd) {
    m_hwnd = hWnd;

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount       = 2;
    sd.BufferDesc.Width  = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags            = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow     = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed         = TRUE;
    sd.SwapEffect       = DXGI_SWAP_EFFECT_DISCARD;

    UINT createFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags,
        levels, 2, D3D11_SDK_VERSION, &sd,
        &m_swapChain, &m_device, &featureLevel, &m_context);

    if (hr == DXGI_ERROR_UNSUPPORTED) {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createFlags,
            levels, 2, D3D11_SDK_VERSION, &sd,
            &m_swapChain, &m_device, &featureLevel, &m_context);
    }

    if (FAILED(hr)) return false;

    CreateRenderTarget();
    return true;
}

void D3D11Device::Destroy() {
    DestroyRenderTarget();
    if (m_swapChain)  { m_swapChain->Release();  m_swapChain  = nullptr; }
    if (m_context)    { m_context->Release();     m_context    = nullptr; }
    if (m_device)     { m_device->Release();      m_device     = nullptr; }
}

bool D3D11Device::Resize(UINT width, UINT height) {
    if (!m_device || !m_swapChain) return false;
    if (width == 0 || height == 0) return true;  // 最小化时不销毁渲染目标，恢复后再重建
    DestroyRenderTarget();
    HRESULT hr = m_swapChain->ResizeBuffers(0, width, height,
                                            DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) return false;
    CreateRenderTarget();
    return true;
}

void D3D11Device::BeginFrame(const float clearColor[4]) {
    if (!m_context || !m_renderTargetView) return;
    m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
    m_context->ClearRenderTargetView(m_renderTargetView, clearColor);
}

void D3D11Device::EndFrame(bool vsync) {
    if (m_swapChain)
        m_swapChain->Present(vsync ? 1 : 0, 0);
}

void D3D11Device::CreateRenderTarget() {
    if (m_renderTargetView) return;  // 已存在，避免重复创建
    ID3D11Texture2D* backBuffer = nullptr;
    if (SUCCEEDED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
        if (FAILED(m_device->CreateRenderTargetView(backBuffer, nullptr,
                                                    &m_renderTargetView)))
            m_renderTargetView = nullptr;
        backBuffer->Release();
    }
}

void D3D11Device::DestroyRenderTarget() {
    if (m_renderTargetView) {
        m_renderTargetView->Release();
        m_renderTargetView = nullptr;
    }
}

} // namespace coco
