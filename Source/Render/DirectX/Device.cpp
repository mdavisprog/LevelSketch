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

#include "Device.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Window.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "Adapter.hpp"
#include "CommandAllocator.hpp"
#include "CommandQueue.hpp"
#include "DescriptorHeap.hpp"
#include "RootSignature.hpp"
#include "SwapChain.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

Device::Device()
{
}

Device::~Device()
{
}

bool Device::Initialize()
{
    m_Adapter = UniquePtr<Adapter>::New();
    if (!m_Adapter->Initialize())
    {
        return false;
    }

    HRESULT Result { D3D12CreateDevice(m_Adapter->GetAdapter(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create device. Error: %s", Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    m_CommandQueue = UniquePtr<CommandQueue>::New();
    if (!m_CommandQueue->Initialize(this))
    {
        return false;
    }

    m_CommandAllocator = UniquePtr<CommandAllocator>::New();
    if (!m_CommandAllocator->Initialize(this))
    {
        return false;
    }

    if (!CreateHeaps())
    {
        return false;
    }

    m_RootSignature = UniquePtr<RootSignature>::New();
    if (!m_RootSignature->Initialize(this))
    {
        return false;
    }

    return true;
}

ID3D12Device9* Device::Get() const
{
    return m_Device.Get();
}

Adapter const* Device::GetAdapter() const
{
    return m_Adapter.Get();
}

CommandAllocator const* Device::GetCommandAllocator() const
{
    return m_CommandAllocator.Get();
}

CommandQueue const* Device::GetCommandQueue() const
{
    return m_CommandQueue.Get();
}

RootSignature const* Device::GetRootSignature() const
{
    return m_RootSignature.Get();
}

DescriptorHeap* Device::SRVHeap() const
{
    return m_SRV.Get();
}

DescriptorHeap* Device::DSVHeap() const
{
    return m_DSV.Get();
}

u64 Device::MaxSRVDescriptors() const
{
    return m_MaxSRVDescriptors;
}

bool Device::CreateHeaps()
{
    m_DSV = UniquePtr<DescriptorHeap>::New();
    if (!m_DSV->Initialize(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1))
    {
        return false;
    }

    m_SRV = UniquePtr<DescriptorHeap>::New();
    if (!m_SRV->Initialize(this,
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            m_MaxSRVDescriptors,
            D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE))
    {
        return false;
    }

    return true;
}

}
}
}
