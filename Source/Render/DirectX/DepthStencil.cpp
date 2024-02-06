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

#include "DepthStencil.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "DescriptorHeap.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

DepthStencil::DepthStencil()
{
}

bool DepthStencil::Initialize(Device const* Device_, const Vector2i& Size)
{
    m_Heap = UniquePtr<DescriptorHeap>::New();
    if (!m_Heap->Initialize(Device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1))
    {
        return false;
    }

    D3D12_HEAP_PROPERTIES HeapProps {};
    HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC ResourceDesc {};
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Width = Size.X;
    ResourceDesc.Height = Size.Y;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 0;
    ResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc {};
    DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    DSVDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CLEAR_VALUE ClearValue {};
    ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    ClearValue.DepthStencil.Depth = 1.0f;
    ClearValue.DepthStencil.Stencil = 0;

    HRESULT Result { Device_->Get()->CreateCommittedResource(&HeapProps,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &ClearValue,
        IID_PPV_ARGS(&m_Resource)) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create committed resource for depth stencil. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    Device_->Get()->CreateDepthStencilView(m_Resource.Get(), &DSVDesc, m_Heap->CPUOffset(0));

    return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencil::CPUOffset() const
{
    return m_Heap->CPUOffset(0);
}

}
}
}
