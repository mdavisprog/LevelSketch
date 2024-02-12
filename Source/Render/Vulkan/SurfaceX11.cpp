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

#include "SurfaceX11.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Window.hpp"
#include "Errors.hpp"
#include "Loader.hpp"

#if defined(PLATFORM_SDL2)
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#endif

#include <X11/Xlib.h>

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

SurfaceX11::SurfaceX11()
    : Surface()
{
}

SurfaceX11::~SurfaceX11()
{
}

VkSurfaceKHR SurfaceX11::InternalInitialize(VkInstance Instance, Platform::Window* PlatformWindow)
{
    Window Window_;
    Display* Display_ { nullptr };
#if defined(PLATFORM_SDL2)
    SDL_Window* Handle { reinterpret_cast<SDL_Window*>(PlatformWindow->Handle()) };
    SDL_SysWMinfo Info {};
    SDL_GetVersion(&Info.version);
    if (SDL_GetWindowWMInfo(Handle, &Info) != SDL_TRUE)
    {
        Core::Console::Error("Failed to retrieve native window information. Error: %s", SDL_GetError());
        return VK_NULL_HANDLE;
    }

    Window_ = Info.info.x11.window;
    Display_ = Info.info.x11.display;

    Vector2i Resolution {};
    SDL_GetWindowSizeInPixels(Handle, &Resolution.X, &Resolution.Y);
    SetResolution(Resolution);
#else
#error "SurfaceX11::InternalInitialize needs platform specific implementation!"
#endif

    PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHRFn {
        Loader::Instance().LoadFn<PFN_vkCreateXlibSurfaceKHR>(Instance, "vkCreateXlibSurfaceKHR")
    };

    if (vkCreateXlibSurfaceKHRFn == nullptr)
    {
        Core::Console::Error("Failed to find function vkCreateXlibSurfaceKHR.");
        return VK_NULL_HANDLE;
    }

    VkXlibSurfaceCreateInfoKHR CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    CreateInfo.dpy = Display_;
    CreateInfo.window = Window_;

    VkSurfaceKHR SurfaceResult { VK_NULL_HANDLE };
    VkResult Result { vkCreateXlibSurfaceKHRFn(Instance, &CreateInfo, nullptr, &SurfaceResult) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create X11 surface. Error: %s", Errors::ToString(Result));
        return VK_NULL_HANDLE;
    }

    return SurfaceResult;
}

}
}
}
