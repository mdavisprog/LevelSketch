/**

MIT License

Copyright (c) 2023 Mitchell Davis <mdavisprog@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include "../Renderer.hpp"
#include "RenderBuffer.hpp"
#include "../../Platform/Windows/Common.hpp"

#include <d3d12.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

#define FRAME_COUNT 2

class Renderer : public LevelSketch::Render::Renderer
{
public:
    Renderer();

    virtual bool Initialize() override;
    virtual bool Initialize(Platform::Window* Window) override;
    virtual void Shutdown() override;
    virtual void Render(Platform::Window* Window) override;

private:
    bool LoadPipeline(Platform::Window* Window);
    bool LoadAssets(Platform::Window* Window);
    IDXGIAdapter1* GetHardwareAdapter(IDXGIFactory1* Factory) const;
    void WaitForPreviousFrame();

    UINT m_FrameIndex { 0 };
    Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_RenderTargets[FRAME_COUNT];
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1> m_CommandList;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    UINT m_HeapDescriptorSize { 0 };
    UINT64 m_FenceValue { 0 };
    HANDLE m_FenceEvent { nullptr };

    RenderBuffer m_RenderBuffer {};
};

}
}
}
