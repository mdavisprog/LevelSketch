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

#include "CommandList.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "CommandAllocator.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

CommandList::CommandList()
{
}

bool CommandList::Initialize(Device const* Device_, CommandAllocator* Allocator)
{
    HRESULT Result { Device_->Get()->CreateCommandList(0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        Allocator->Get(),
        nullptr,
        IID_PPV_ARGS(&m_Commands)) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create command list. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    m_Allocator = Allocator;

    return true;
}

bool CommandList::Close() const
{
    HRESULT Result { m_Commands->Close() };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to close command list. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    return true;
}

bool CommandList::Reset() const
{
    HRESULT Result { m_Commands->Reset(m_Allocator->Get(), nullptr) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to reset command list. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    return true;
}

ID3D12GraphicsCommandList1* CommandList::Get() const
{
    return m_Commands.Get();
}

}
}
}
