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

#include <vulkan/vulkan.hpp>

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class CommandPool;
class Device;

class Texture final
{
private:
    static u32 s_ID;

public:
    static VkImageView CreateView(Device const* Device_, VkImage Image, VkFormat Format);

    Texture();

    bool Initialize(Device const* Device_,
        CommandPool const* Pool,
        const void* Data,
        u32 Width,
        u32 Height,
        u8 BytesPerPixel);
    void Shutdown(Device const* Device_);

    VkImage Get() const;
    u32 ID() const;

private:
    void Transition(Device const* Device_, CommandPool const* Pool, VkImageLayout From, VkImageLayout) const;

    VkImage m_Image { VK_NULL_HANDLE };
    VkImageView m_ImageView { VK_NULL_HANDLE };
    VkDeviceMemory m_Memory { VK_NULL_HANDLE };
    u32 m_ID { 0 };
};

}
}
}
