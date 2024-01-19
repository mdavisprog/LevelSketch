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

#define VK_USE_PLATFORM_XLIB_KHR

#include "Surface.hpp"
#include "../../Core/Console.hpp"
#include "Errors.hpp"
#include "Loader.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Surface::Surface()
{
}

bool Surface::Initialize(VkInstance Instance, Window Window_, Display* Display_)
{
    PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHRFn {
            Loader::Instance().LoadFn<PFN_vkCreateXlibSurfaceKHR>(Instance, "vkCreateXlibSurfaceKHR")
    };

    if (vkCreateXlibSurfaceKHRFn == nullptr)
    {
        Core::Console::Error("Failed to find function vkCreateXlibSurfaceKHR.");
        return false;
    }

    VkXlibSurfaceCreateInfoKHR CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    CreateInfo.dpy = Display_;
    CreateInfo.window = Window_;

    VkResult Result { vkCreateXlibSurfaceKHRFn(Instance, &CreateInfo, nullptr, &m_Surface) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create X11 surface. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void Surface::Shutdown(VkInstance Instance)
{
    if (m_Surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(Instance, m_Surface, nullptr);
        m_Surface = VK_NULL_HANDLE;
    }
}

bool Surface::IsValid() const
{
    return m_Surface != VK_NULL_HANDLE;
}

VkSurfaceKHR Surface::Handle() const
{
    return m_Surface;
}

}
}
}
