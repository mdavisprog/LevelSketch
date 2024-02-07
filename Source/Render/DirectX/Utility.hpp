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

#include <d3d12.h>

namespace LevelSketch
{
namespace Render
{

enum class VertexFormat;

namespace DirectX
{
namespace Utility
{

DXGI_FORMAT ToDXGIFormat(const VertexFormat& Format);

D3D12_HEAP_PROPERTIES MakeHeapProperties(D3D12_HEAP_TYPE Type = D3D12_HEAP_TYPE_DEFAULT);
D3D12_RESOURCE_DESC MakeResourceDescription(D3D12_RESOURCE_DIMENSION Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN);

D3D12_RESOURCE_BARRIER MakeResourceBarrierTransition(ID3D12Resource* Resource,
    D3D12_RESOURCE_STATES Before,
    D3D12_RESOURCE_STATES After,
    UINT Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
    D3D12_RESOURCE_BARRIER_FLAGS Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

}
}
}
}
