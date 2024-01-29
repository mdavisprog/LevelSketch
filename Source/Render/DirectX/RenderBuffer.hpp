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

#include "../../Core/Types.hpp"

#include <d3d12.h>
#include <wrl/client.h>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

class RenderBuffer
{
public:
    RenderBuffer();

    bool Initialize(ID3D12Device* Device, u64 VertexBufferSize, u64 IndexBufferSize);
    bool Initialized() const;

    RenderBuffer& SetStride(u32 Stride);
    RenderBuffer& SetFormat(DXGI_FORMAT Format);

    bool UploadVertexData(const void* Source, u64 Size);
    bool UploadIndexData(const void* Source, u64 Size);
    void BindViews(ID3D12GraphicsCommandList* CommandList) const;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer {};
    Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer {};

    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView { 0 };
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView { 0 };
};

}
}
}
