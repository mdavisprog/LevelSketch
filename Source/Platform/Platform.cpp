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

#include "Platform.hpp"
#include "../Core/Memory/UniquePtr.hpp"
#include "Window.hpp"

#include <utility>

namespace LevelSketch
{
namespace Platform
{

const UniquePtr<Platform>& Platform::Instance()
{
    static UniquePtr<Platform> Instance { CreateInstance() };
    return Instance;
}

int Platform::Run()
{
    if (m_OnFrame == nullptr)
    {
        return -1;
    }

    while (true)
    {
        if (!RunFrame())
        {
            break;
        }

        // TODO: Sleep if there is extra time remaining.
    }

    return 0;
}

bool Platform::RunFrame()
{
    if (m_OnFrame == nullptr)
    {
        return false;
    }

    UpdateTimingData(m_TimingData);
    m_TimingData.TotalTimeSeconds += m_TimingData.DeltaSeconds;

    if (!m_OnFrame(m_TimingData))
    {
        return false;
    }

    return true;
}

TimingData Platform::GetTimingData() const
{
    return m_TimingData;
}

Platform& Platform::SetOnFrame(OnFrameSignature&& Fn)
{
    m_OnFrame = std::move(Fn);
    return *this;
}

Window* Platform::NewWindow(const WindowDescription& Description)
{
    UniquePtr<Window> Result { InternalNewWindow() };

    if (Result == nullptr)
    {
        return nullptr;
    }

    if (!Result->Create(Description))
    {
        return nullptr;
    }

    m_Windows.Push(std::move(Result));

    return m_Windows.Back().Get();
}

Platform& Platform::CloseWindow(Window* Window)
{
    if (Window == nullptr)
    {
        return *this;
    }

    Window->Close();

    for (const UniquePtr<LevelSketch::Platform::Window>& Item : m_Windows)
    {
        if (Item == Window)
        {
            m_Windows.Remove(Item);
            break;
        }
    }

    return *this;
}

const Array<UniquePtr<Window>>& Platform::Windows() const
{
    return m_Windows;
}

u64 Platform::WindowCount() const
{
    return m_Windows.Size();
}

bool Platform::HasWindow(Window* Window) const
{
    for (const UniquePtr<LevelSketch::Platform::Window>& Win : m_Windows)
    {
        if (Win.Get() == Window)
        {
            return true;
        }
    }

    return false;
}

void Platform::UpdateTimingData(TimingData&)
{
}

}
}
