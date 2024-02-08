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
#include "CommandBuffer.hpp"
#include "Device.hpp"

#import <Metal/Metal.h>

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

CommandQueue::CommandQueue()
{
}

CommandQueue::~CommandQueue()
{
}

bool CommandQueue::Initialize(Device const* Device_)
{
    m_CommandQueue = [Device_->Get() newCommandQueue];

    if (m_CommandQueue == nullptr)
    {
        Core::Console::Error("Failed to create command queue.");
        return false;
    }

    return true;
}

id<MTLCommandQueue> CommandQueue::Get() const
{
    return m_CommandQueue;
}

CommandBuffer* CommandQueue::BeginBuffer()
{
    if (m_CommandBuffer == nullptr)
    {
        m_CommandBuffer = UniquePtr<CommandBuffer>::New();

        if (!m_CommandBuffer->Initialize(this))
        {
            m_CommandBuffer = nullptr;
        }
    }

    return m_CommandBuffer.Get();
}

CommandBuffer* CommandQueue::CurrentBuffer() const
{
    return m_CommandBuffer.Get();
}

void CommandQueue::EndBuffer()
{
    if (m_CommandBuffer == nullptr)
    {
        return;
    }

    m_CommandBuffer->EndEncoding();
    m_CommandBuffer = nullptr;
}

}
}
}
