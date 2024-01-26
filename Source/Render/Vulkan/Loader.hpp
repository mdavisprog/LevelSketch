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

#include "vulkan/vulkan.hpp"

namespace LevelSketch
{

namespace Core::Containers
{
template<typename T>
class Array;
}

template<typename T>
using Array = Core::Containers::Array<T>;

namespace Render
{
namespace Vulkan
{

class Loader
{
public:
    static Loader& Instance();

    bool Initialize();
    bool IsInitialized() const;
    void Shutdown();

    template<typename T>
    T LoadFn(VkInstance Instance, const char* Name)
    {
        if (!LoadGetInstanceProcAddr())
        {
            return nullptr;
        }

        return reinterpret_cast<T>(m_GetInstanceProcAddr(Instance, Name));
    }

    bool GetInstanceExtensionProperties(Array<VkExtensionProperties>& Properties);

private:
    Loader();

    bool LoadGetInstanceProcAddr();

    void* m_Handle { nullptr };
    PFN_vkGetInstanceProcAddr m_GetInstanceProcAddr { nullptr };
};

}
}
}
