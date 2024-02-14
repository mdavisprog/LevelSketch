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

#include "Sampler.hpp"
#include "../../Core/Console.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Sampler::Sampler()
{
}

bool Sampler::Initialize(Device const* Device_)
{
    VkSamplerCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    CreateInfo.minFilter = VK_FILTER_LINEAR;
    CreateInfo.magFilter = VK_FILTER_LINEAR;
    CreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateInfo.anisotropyEnable = VK_TRUE;
    CreateInfo.maxAnisotropy = Device_->GetPhysicalDevice()->GetInfo().MaxSamplerAnisotropy;
    CreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    CreateInfo.unnormalizedCoordinates = VK_FALSE;
    CreateInfo.compareEnable = VK_FALSE;
    CreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    CreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    CreateInfo.mipLodBias = 0.0f;
    CreateInfo.minLod = 0.0f;
    CreateInfo.maxLod = 0.0f;

    VkResult Result { vkCreateSampler(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_Sampler) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create sampler. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void Sampler::Shutdown(Device const* Device_)
{
    if (m_Sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(Device_->GetLogicalDevice()->Get(), m_Sampler, nullptr);
        m_Sampler = VK_NULL_HANDLE;
    }
}

VkSampler Sampler::Get() const
{
    return m_Sampler;
}

}
}
}
