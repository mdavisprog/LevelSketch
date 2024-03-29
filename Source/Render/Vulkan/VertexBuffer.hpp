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

#include "../../Core/Memory/UniquePtr.hpp"
#include "../../Core/Types.hpp"
#include "../Handle.hpp"

#include <vulkan/vulkan.hpp>

namespace LevelSketch
{
namespace Render
{

struct VertexBufferDescription;

namespace Vulkan
{

class Buffer;
class Device;

class VertexBuffer final
{
public:
    VertexBuffer();

    bool Initialize(Device const* Device_, const VertexBufferDescription& Description);
    void Shutdown(Device const* Device_);

    Buffer const* GetVertexBuffer() const;
    Buffer const* GetIndexBuffer() const;

    VkIndexType IndexType() const;
    VertexBufferHandle Handle() const;

private:
    bool InitializeBuffer(const UniquePtr<Buffer>& Buffer_,
        Device const* Device_,
        u64 Size,
        VkBufferUsageFlags Usage) const;

    UniquePtr<Buffer> m_VertexBuffer { nullptr };
    UniquePtr<Buffer> m_IndexBuffer { nullptr };
    VkIndexType m_IndexType { VK_INDEX_TYPE_UINT32 };
    VertexBufferHandle m_Handle {};
};

}
}
}
