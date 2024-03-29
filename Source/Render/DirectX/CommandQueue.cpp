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

#include "CommandQueue.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

CommandQueue::CommandQueue()
{
}

bool CommandQueue::Initialize(Device const* Device_)
{
    D3D12_COMMAND_QUEUE_DESC CommandQueueDescription { 0 };
    CommandQueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    CommandQueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HRESULT Result { Device_->Get()->CreateCommandQueue(&CommandQueueDescription, IID_PPV_ARGS(&m_CommandQueue)) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create command queue. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    return true;
}

bool CommandQueue::Signal(ID3D12Fence* Fence, u64 Value) const
{
    HRESULT Result { m_CommandQueue->Signal(Fence, Value) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to signal command queue. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    return true;
}

void CommandQueue::Execute(u32 Count, ID3D12CommandList* const* Lists) const
{
    m_CommandQueue->ExecuteCommandLists(Count, Lists);
}

ID3D12CommandQueue* CommandQueue::Get() const
{
    return m_CommandQueue.Get();
}

}
}
}
