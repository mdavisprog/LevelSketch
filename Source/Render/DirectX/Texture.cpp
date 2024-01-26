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

#include "Texture.hpp"
#include "../../Core/Assert.hpp"
#include "../../Core/Console.hpp"
#include "Utility.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

u32 Texture::s_ID { 0 };

Texture::Texture()
{
}

bool Texture::Create(ID3D12Device* Device, u32 Width, u32 Height)
{
    if (Initialized())
    {
        return true;
    }

    D3D12_HEAP_PROPERTIES HeapProperties { Utility::MakeHeapProperties() };

    D3D12_RESOURCE_DESC ResourceDescription = { Utility::MakeResourceDescription(D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        DXGI_FORMAT_R8G8B8A8_UNORM) };
    ResourceDescription.Width = Width;
    ResourceDescription.Height = Height;

    HRESULT Result = Device->CreateCommittedResource(&HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDescription,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_Texture));

    if (Result != S_OK)
    {
        Core::Console::Warning("Failed to create texture resource! Error Code: 0x%08X", Result);
        return false;
    }

    m_Width = Width;
    m_Height = Height;

    return true;
}

bool Texture::Upload(ID3D12GraphicsCommandList* CommandList,
    ID3D12DescriptorHeap* Heap,
    u64 Offset,
    const void* Data,
    Microsoft::WRL::ComPtr<ID3D12Resource>& UploadResource)
{
    LS_ASSERT(Initialized());

    if (CommandList == nullptr)
    {
        return false;
    }

    u64 RequiredSize { 0 };
    ID3D12Device* Device { nullptr };
    const D3D12_RESOURCE_DESC TextureDesc { m_Texture->GetDesc() };
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts { 0 };
    u32 NumRows { 0 };
    u64 RowSizeInBytes { 0 };
    m_Texture->GetDevice(__uuidof(ID3D12Device), reinterpret_cast<void**>(&Device));
    Device->GetCopyableFootprints(&TextureDesc, 0, 1, 0, &Layouts, &NumRows, &RowSizeInBytes, &RequiredSize);

    D3D12_HEAP_PROPERTIES HeapProperties { Utility::MakeHeapProperties(D3D12_HEAP_TYPE_UPLOAD) };
    D3D12_RESOURCE_DESC UploadResourceDescription { Utility::MakeResourceDescription() };
    UploadResourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    UploadResourceDescription.Width = RequiredSize;
    HRESULT Result = Device->CreateCommittedResource(&HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &UploadResourceDescription,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&UploadResource));

    if (Result != S_OK)
    {
        Core::Console::Warning("Failed to create texture upload resource!");
        return false;
    }

    D3D12_SUBRESOURCE_DATA TextureData = { 0 };
    TextureData.pData = Data;
    TextureData.RowPitch = m_Width * 4;
    TextureData.SlicePitch = TextureData.RowPitch * m_Height;

    u8* UploadBuffer { nullptr };
    if (UploadResource->Map(0, nullptr, reinterpret_cast<void**>(&UploadBuffer)) != S_OK)
    {
        Core::Console::Warning("Failed to map to upload resource memory!");
        UploadResource.Reset();
        return false;
    }

    D3D12_MEMCPY_DEST Memcpy { UploadBuffer + Layouts.Offset,
        Layouts.Footprint.RowPitch,
        static_cast<u64>(Layouts.Footprint.RowPitch) * static_cast<u64>(NumRows) };
    for (u32 Slice = 0; Slice < Layouts.Footprint.Depth; Slice++)
    {
        u8* Dest { static_cast<u8*>(Memcpy.pData) + Memcpy.SlicePitch * Slice };
        const u8* Src { static_cast<const u8*>(TextureData.pData) + TextureData.SlicePitch * static_cast<u64>(Slice) };

        for (u32 Row = 0; Row < NumRows; Row++)
        {
            memcpy(Dest + Memcpy.RowPitch * Row, Src + TextureData.RowPitch * static_cast<u64>(Row), RowSizeInBytes);
        }
    }

    UploadResource->Unmap(0, nullptr);

    D3D12_TEXTURE_COPY_LOCATION CopyDest { m_Texture.Get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };
    D3D12_TEXTURE_COPY_LOCATION CopySrc { UploadResource.Get(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, Layouts };
    CommandList->CopyTextureRegion(&CopyDest, 0, 0, 0, &CopySrc, nullptr);

    D3D12_RESOURCE_BARRIER Barrier { Utility::MakeResourceBarrierTransition(m_Texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) };
    CommandList->ResourceBarrier(1, &Barrier);

    D3D12_SHADER_RESOURCE_VIEW_DESC ShaderViewDesc { 0 };
    ShaderViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    ShaderViewDesc.Format = TextureDesc.Format;
    ShaderViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    ShaderViewDesc.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor { Heap->GetCPUDescriptorHandleForHeapStart() };
    CPUDescriptor.ptr += Offset;
    Device->CreateShaderResourceView(m_Texture.Get(), &ShaderViewDesc, CPUDescriptor);

    Device->Release();

    m_ID = ++s_ID;
    m_Offset = Offset;

    return true;
}

bool Texture::Initialized() const
{
    return m_Texture != nullptr;
}

u32 Texture::ID() const
{
    return m_ID;
}

u64 Texture::Offset() const
{
    return m_Offset;
}

}
}
}
