/**

MIT License

Copyright (c) 2024 Mitchell Davis <mdavisprog@gmail.com>

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

#include "Viewport.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "DescriptorHeap.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

Viewport::Viewport()
{
}

Viewport::~Viewport()
{
}

bool Viewport::Initialize(Platform::Window* Window, Device const* Device_, u32 BufferCount)
{
    m_RTV = UniquePtr<DescriptorHeap>::New();
    if (!m_RTV->Initialize(Device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, BufferCount))
    {
        return false;
    }

    m_SwapChain = UniquePtr<SwapChain>::New();
    if (!m_SwapChain->Initialize(Window, Device_, BufferCount))
    {
        return false;
    }

    m_FrameIndex = m_SwapChain->BackBufferIndex();
    m_RenderTargets.Resize(BufferCount);

    for (u32 Index = 0; Index < m_RenderTargets.Size(); Index++)
    {
        HRESULT Result { m_SwapChain->Get()->GetBuffer(Index, IID_PPV_ARGS(&m_RenderTargets[Index])) };

        if (FAILED(Result))
        {
            Core::Console::Error("Failed to get buffer for swap chain at index %d. Error: %s",
                Index,
                Platform::Windows::Errors::ToString(Result).Data());
            return false;
        }

        Device_->Get()->CreateRenderTargetView(m_RenderTargets[Index].Get(), nullptr, m_RTV->CPUOffset(Index));
    }

    m_Window = Window;
    return true;
}

void Viewport::UpdateFrameIndex()
{
    m_FrameIndex = m_SwapChain->BackBufferIndex();
}

bool Viewport::Present(u32 SyncInterval, u32 Flags) const
{
    return m_SwapChain->Present(SyncInterval, Flags);
}

Platform::Window* Viewport::GetWindow() const
{
    return m_Window;
}

D3D12_CPU_DESCRIPTOR_HANDLE Viewport::RTVCPUOffset() const
{
    return m_RTV->CPUOffset(m_FrameIndex);
}

ID3D12Resource* Viewport::CurrentRenderTarget() const
{
    return m_RenderTargets[m_FrameIndex].Get();
}

}
}
}
