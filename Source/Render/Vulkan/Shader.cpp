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

#include "Shader.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Containers/Array.hpp"
#include "../../Platform/FileSystem.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "LogicalDevice.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Shader::Shader()
{
}

bool Shader::Load(Device const* Device_, const char* Path)
{
    Array<u8> Data { Platform::FileSystem::ReadBinaryContents(Path) };

    if (Data.IsEmpty())
    {
        Core::Console::Error("Failed to load shader. No contents were read from file '%s'.", Path);
        return false;
    }

    VkShaderModuleCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = Data.Size();
    CreateInfo.pCode = reinterpret_cast<u32*>(Data.Data());

    VkResult Result { vkCreateShaderModule(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_Module) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create shader module. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void Shader::Shutdown(Device const* Device_)
{
    if (m_Module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(Device_->GetLogicalDevice()->Get(), m_Module, nullptr);
        m_Module = VK_NULL_HANDLE;
    }
}

VkShaderModule Shader::Get() const
{
    return m_Module;
}

}
}
}
