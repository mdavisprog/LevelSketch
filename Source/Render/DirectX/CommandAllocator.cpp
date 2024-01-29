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

#include "CommandAllocator.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

CommandAllocator::CommandAllocator()
{
}

bool CommandAllocator::Initialize(Device const* Device_)
{
    HRESULT Result { Device_->Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&m_Allocator)) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create command allocator. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    return true;
}

bool CommandAllocator::Reset() const
{
    HRESULT Result { m_Allocator->Reset() };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to reset command allocator. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    return true;
}

ID3D12CommandAllocator* CommandAllocator::Get() const
{
    return m_Allocator.Get();
}

}
}
}
