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

#pragma once

#include "../../Core/Containers/Array.hpp"
#include "../../Core/Memory/UniquePtr.hpp"
#include <d3d12.h>
#include <wrl/client.h>

namespace LevelSketch
{

namespace Platform
{
class Window;
}

namespace Render
{
namespace DirectX
{

class DescriptorHeap;
class Device;
class SwapChain;

class Viewport
{
public:
    Viewport();

    bool Initialize(Platform::Window* Window, Device const* Device_, u32 BufferCount);
    void UpdateFrameIndex();
    bool Present(u32 SyncInterval, u32 Flags) const;

    Platform::Window* GetWindow() const;
    D3D12_CPU_DESCRIPTOR_HANDLE RTVCPUOffset() const;
    ID3D12Resource* CurrentRenderTarget() const;

private:
    Platform::Window* m_Window { nullptr };
    UniquePtr<SwapChain> m_SwapChain { nullptr };
    UniquePtr<DescriptorHeap> m_RTV { nullptr };
    Array<Microsoft::WRL::ComPtr<ID3D12Resource>> m_RenderTargets {};
    u32 m_FrameIndex { 0 };
};

}
}
}
