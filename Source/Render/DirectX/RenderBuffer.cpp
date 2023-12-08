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

#include "RenderBuffer.hpp"
#include "../../Core/Assert.hpp"
#include "../../Core/Console.hpp"
#include "Utility.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

RenderBuffer::RenderBuffer()
{
}

bool RenderBuffer::Initialize(ID3D12Device* Device, u64 VertexBufferSize, u64 IndexBufferSize)
{
    if (Initialized())
    {
        return true;
    }

    if (Device == nullptr)
    {
        return false;
    }

    D3D12_HEAP_PROPERTIES HeapProperties { Utility::MakeHeapProperties(D3D12_HEAP_TYPE_UPLOAD) };
    D3D12_RESOURCE_DESC ResourceDescription { Utility::MakeResourceDescription() };
    ResourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDescription.Width = VertexBufferSize;
    HRESULT Result = Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDescription,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_VertexBuffer)
    );

    if (Result != S_OK)
    {
        Core::Console::Error("Failed to create committed resource for vertex buffer! Error Code: 0x%08X", Result);
        return false;
    }

    ResourceDescription.Width = IndexBufferSize;
    Result = Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDescription,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_IndexBuffer)
    );

    if (Result != S_OK)
    {
        Core::Console::Error("Failed to create committed resource for index buffer! Error Code: %d", Result);
        return false;
    }

    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

    return true;
}

bool RenderBuffer::Initialized() const
{
    return m_VertexBuffer != nullptr && m_IndexBuffer != nullptr;
}

RenderBuffer& RenderBuffer::SetStride(u32 Stride)
{
    m_VertexBufferView.StrideInBytes = Stride;
    return *this;
}

RenderBuffer& RenderBuffer::SetFormat(DXGI_FORMAT Format)
{
    m_IndexBufferView.Format = Format;
    return *this;
}

bool RenderBuffer::UploadVertexData(const void* Source, u64 Size)
{
    LS_ASSERT(Initialized());

    u8* Data { nullptr };
    D3D12_RANGE Range { 0, 0 };
    if (m_VertexBuffer->Map(0, &Range, reinterpret_cast<void**>(&Data)) != S_OK)
    {
        Core::Console::Warning("Failed to map vertex buffer memory!");
        return false;
    }

    memcpy(Data, Source, Size);

    m_VertexBuffer->Unmap(0, nullptr);
    m_VertexBufferView.SizeInBytes = static_cast<u32>(Size);
    return true;
}

bool RenderBuffer::UploadIndexData(const void* Source, u64 Size)
{
    LS_ASSERT(Initialized());

    u8* Data { nullptr };
    D3D12_RANGE Range { 0, 0 };
    if (m_IndexBuffer->Map(0, &Range, reinterpret_cast<void**>(&Data)) != S_OK)
    {
        Core::Console::Warning("Failed to map vertex buffer memory!");
        return false;
    }

    memcpy(Data, Source, Size);

    m_IndexBuffer->Unmap(0, nullptr);
    m_IndexBufferView.SizeInBytes = static_cast<u32>(Size);
    return true;
}

void RenderBuffer::BindViews(ID3D12GraphicsCommandList* CommandList) const
{
    LS_ASSERT(Initialized());

    if (CommandList == nullptr)
    {
        return;
    }

    CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    CommandList->IASetIndexBuffer(&m_IndexBufferView);
}

}
}
}
