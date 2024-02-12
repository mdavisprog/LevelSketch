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

#include "Surface.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Surface::Surface()
{
}

Surface::~Surface()
{
}

bool Surface::Initialize(VkInstance Instance, Platform::Window* Window)
{
    m_Surface = InternalInitialize(Instance, Window);
    return m_Surface != VK_NULL_HANDLE;
}

void Surface::Shutdown(VkInstance Instance)
{
    if (m_Surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(Instance, m_Surface, nullptr);
        m_Surface = VK_NULL_HANDLE;
    }
}

VkSurfaceKHR Surface::Get() const
{
    return m_Surface;
}

Vector2i Surface::Resolution() const
{
    return m_Resolution;
}

void Surface::SetResolution(const Vector2i& Resolution)
{
    m_Resolution = Resolution;
}

}
}
}
