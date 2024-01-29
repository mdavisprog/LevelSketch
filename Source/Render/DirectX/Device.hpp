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
#include "../Renderer.hpp"
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

class Adapter;
class CommandAllocator;
class CommandQueue;
class DescriptorHeap;
class RootSignature;
class SwapChain;

class Device
{
public:
    Device();
    ~Device();

    bool Initialize();
    bool Initialize(Platform::Window* Window);

    ID3D12Device9* Get() const;
    Adapter const* GetAdapter() const;
    CommandAllocator const* GetCommandAllocator() const;
    CommandQueue const* GetCommandQueue() const;
    RootSignature const* GetRootSignature() const;

    SwapChain const* GetSwapChain(Platform::Window* Window) const;
    SwapChain const* FirstSwapChain() const;
    u64 NumSwapChains() const;

    DescriptorHeap* RTVHeap() const;
    DescriptorHeap* SRVHeap() const;
    DescriptorHeap* DSVHeap() const;

    u64 MaxSRVDescriptors() const;

private:
    bool CreateHeaps();

    Microsoft::WRL::ComPtr<ID3D12Device9> m_Device { nullptr };
    UniquePtr<Adapter> m_Adapter { nullptr };
    UniquePtr<CommandAllocator> m_CommandAllocator { nullptr };
    UniquePtr<CommandQueue> m_CommandQueue { nullptr };
    UniquePtr<RootSignature> m_RootSignature { nullptr };
    Array<UniquePtr<SwapChain>> m_SwapChains {};

    UniquePtr<DescriptorHeap> m_RTV { nullptr };
    UniquePtr<DescriptorHeap> m_DSV { nullptr };
    UniquePtr<DescriptorHeap> m_SRV { nullptr };

    u32 m_BufferCount { 2 };
    u32 m_MaxSRVDescriptors { 100 };
};

}
}
}
