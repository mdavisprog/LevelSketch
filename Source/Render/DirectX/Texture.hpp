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
#include "../Handle.hpp"

#include <d3d12.h>
#include <wrl/client.h>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

class Texture
{
public:
    Texture();

    bool Create(ID3D12Device* Device, u32 Width, u32 Height);
    bool Upload(ID3D12GraphicsCommandList* CommandList,
        ID3D12DescriptorHeap* Heap,
        u64 Offset,
        const void* Data,
        Microsoft::WRL::ComPtr<ID3D12Resource>& UploadResource);

    bool Initialized() const;
    TextureHandle Handle() const;
    u64 Offset() const;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture {};
    u32 m_Width { 0 };
    u32 m_Height { 0 };
    u64 m_Offset { 0 };
    TextureHandle m_Handle {};
};

}
}
}
