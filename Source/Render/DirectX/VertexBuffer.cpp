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

#include "VertexBuffer.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "../VertexBufferDescription.hpp"
#include "Device.hpp"
#include "Utility.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

u32 VertexBuffer::s_ID { 0 };

VertexBuffer::VertexBuffer()
{
}

bool VertexBuffer::Initialize(Device const* Device_, const VertexBufferDescription& Description)
{
    D3D12_HEAP_PROPERTIES HeapProps {};
    HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC ResourceDesc {};
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Width = Description.VertexBufferSize;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT Result = Device_->Get()->CreateCommittedResource(&HeapProps,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_VertexBuffer));

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create committed resource for vertex buffer! Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    ResourceDesc.Width = Description.IndexBufferSize;
    Result = Device_->Get()->CreateCommittedResource(&HeapProps,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_IndexBuffer));

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create committed resource for index buffer! Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.StrideInBytes = static_cast<u32>(Description.Stride);

    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format =
        Description.IndexFormat == IndexFormatType::U16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    m_ID = ++s_ID;
    return true;
}

bool VertexBuffer::UploadVertexData(const void* Source, u64 Size)
{
    u8* Data { nullptr };
    D3D12_RANGE Range { 0, 0 };

    HRESULT Result { m_VertexBuffer->Map(0, &Range, reinterpret_cast<void**>(&Data)) };

    if (FAILED(Result))
    {
        Core::Console::Warning("Failed to map vertex buffer memory. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    memcpy(Data, Source, Size);

    m_VertexBuffer->Unmap(0, nullptr);
    m_VertexBufferView.SizeInBytes = static_cast<u32>(Size);
    return true;
}

bool VertexBuffer::UploadIndexData(const void* Source, u64 Size)
{
    u8* Data { nullptr };
    D3D12_RANGE Range { 0, 0 };

    HRESULT Result { m_IndexBuffer->Map(0, &Range, reinterpret_cast<void**>(&Data)) };
    if (FAILED(Result))
    {
        Core::Console::Warning("Failed to map index buffer memory. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    memcpy(Data, Source, Size);

    m_IndexBuffer->Unmap(0, nullptr);
    m_IndexBufferView.SizeInBytes = static_cast<u32>(Size);
    return true;
}

void VertexBuffer::BindViews(ID3D12GraphicsCommandList* CommandList) const
{
    CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    CommandList->IASetIndexBuffer(&m_IndexBufferView);
}

u32 VertexBuffer::ID() const
{
    return m_ID;
}

}
}
}
