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

#include "Viewport.hpp"
#include "Device.hpp"
#include "SurfaceX11.hpp"
#include "SwapChain.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Viewport::Viewport()
{
}

Viewport::~Viewport()
{
}

bool Viewport::Initialize(VkInstance Instace, Platform::Window* Window)
{
    m_Surface = UniquePtr<SurfaceX11>::New();

    if (!m_Surface->Initialize(Instace, Window))
    {
        m_Surface = nullptr;
        return false;
    }

    m_Window = Window;
    return true;
}

bool Viewport::InitializeSwapChain(Device const* Device_)
{
    m_SwapChain = UniquePtr<SwapChain>::New();

    if (!m_SwapChain->Initialize(Device_, m_Surface.Get()))
    {
        return false;
    }

    return true;
}

void Viewport::Shutdown(VkInstance Instance, Device const* Device_)
{
    m_Surface->Shutdown(Instance);
    m_SwapChain->Shutdown(Device_);
}

Platform::Window* Viewport::Window() const
{
    return m_Window;
}

Surface const* Viewport::GetSurface() const
{
    return m_Surface.Get();
}

SwapChain* Viewport::GetSwapChain()
{
    return m_SwapChain.Get();
}

SwapChain const* Viewport::GetSwapChain() const
{
    return m_SwapChain.Get();
}

}
}
}
