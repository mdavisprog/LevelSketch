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

#include "UniformBuffer.hpp"
#include "Buffer.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

UniformBuffer::UniformBuffer()
{
}

UniformBuffer::~UniformBuffer()
{
}

bool UniformBuffer::Initialize(Device const* Device_)
{
    m_Buffer = UniquePtr<Buffer>::New();
    if (!m_Buffer->Initialize(Device_,
            sizeof(Uniforms),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        return false;
    }

    if (!m_Buffer->Map(Device_, sizeof(Uniforms)))
    {
        return false;
    }

    UpdateBuffer();

    return true;
}

void UniformBuffer::Shutdown(Device const* Device_)
{
    m_Buffer->Shutdown(Device_);
}

UniformBuffer::Uniforms& UniformBuffer::GetUniforms()
{
    return m_Uniforms;
}

void UniformBuffer::UpdateBuffer() const
{
    m_Buffer->MapData(&m_Uniforms, sizeof(Uniforms));
}

Buffer const* UniformBuffer::Get() const
{
    return m_Buffer.Get();
}

u64 UniformBuffer::Size() const
{
    return sizeof(Uniforms);
}

}
}
}
