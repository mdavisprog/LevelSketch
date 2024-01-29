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

#include "DescriptorHeap.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

static const char* ToString(D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
    switch (Type)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return "CBV_SRV_UAV";
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: return "Sampler";
    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: return "RTV";
    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: return "DSV";
    case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES: return "NUM_TYPES";
    default: break;
    }

    return "Invalid";
}

DescriptorHeap::DescriptorHeap()
{
}

bool DescriptorHeap::Initialize(Device const* Device_,
    D3D12_DESCRIPTOR_HEAP_TYPE Type,
    u32 NumDescriptors,
    D3D12_DESCRIPTOR_HEAP_FLAGS Flags)
{
    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc {};
    HeapDesc.Type = Type;
    HeapDesc.NumDescriptors = NumDescriptors;
    HeapDesc.Flags = Flags;
    HeapDesc.NodeMask = 0;

    HRESULT Result { Device_->Get()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_Heap)) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create descriptor heap of type %s. Error: %s",
            ToString(Type),
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    m_DescriptorSize = Device_->Get()->GetDescriptorHandleIncrementSize(Type);

    return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::CPUOffset(u64 Index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE Result { m_Heap->GetCPUDescriptorHandleForHeapStart() };
    Result.ptr += Index * m_DescriptorSize;
    return Result;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GPUOffset(u64 Index) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE Result { m_Heap->GetGPUDescriptorHandleForHeapStart() };
    Result.ptr += Index * m_DescriptorSize;
    return Result;
}

u64 DescriptorHeap::DescriptorSize() const
{
    return m_DescriptorSize;
}

ID3D12DescriptorHeap* DescriptorHeap::Get() const
{
    return m_Heap.Get();
}

}
}
}
