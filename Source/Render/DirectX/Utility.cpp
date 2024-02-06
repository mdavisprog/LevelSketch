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

#include "Utility.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{
namespace Utility
{

D3D12_HEAP_PROPERTIES MakeHeapProperties(D3D12_HEAP_TYPE Type)
{
    D3D12_HEAP_PROPERTIES Result { 0 };
    Result.Type = Type;
    Result.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    Result.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    Result.CreationNodeMask = 1;
    Result.VisibleNodeMask = 1;
    return Result;
}

D3D12_RESOURCE_DESC MakeResourceDescription(D3D12_RESOURCE_DIMENSION Dimension, DXGI_FORMAT Format)
{
    D3D12_RESOURCE_DESC Result;
    Result.Dimension = Dimension;
    Result.Alignment = 0;
    Result.Width = 0;
    Result.Height = 1;
    Result.DepthOrArraySize = 1;
    Result.MipLevels = 1;
    Result.Format = Format;
    Result.SampleDesc.Count = 1;
    Result.SampleDesc.Quality = 0;
    Result.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    Result.Flags = D3D12_RESOURCE_FLAG_NONE;
    return Result;
}

D3D12_RESOURCE_BARRIER MakeResourceBarrierTransition(ID3D12Resource* Resource,
    D3D12_RESOURCE_STATES Before,
    D3D12_RESOURCE_STATES After,
    UINT Subresource,
    D3D12_RESOURCE_BARRIER_FLAGS Flags)
{
    D3D12_RESOURCE_BARRIER Result { 0 };
    Result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Result.Transition.StateBefore = Before;
    Result.Transition.StateAfter = After;
    Result.Transition.pResource = Resource;
    Result.Transition.Subresource = Subresource;
    Result.Flags = Flags;
    return Result;
}

}
}
}
}
