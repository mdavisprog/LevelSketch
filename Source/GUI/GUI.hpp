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

#include "../Core/Containers/HashMap.hpp"
#include "../Core/Memory/UniquePtr.hpp"
#include "../External/OctaneGUI/VertexBuffer.h"
#include "../Platform/Event.hpp"

namespace OctaneGUI
{
enum class WindowAction : u8;

class Application;
class Event;
class Window;
}

namespace LevelSketch
{

namespace Platform
{
class Window;
}

namespace GUI
{

class GUI
{
public:
    static GUI& Instance();

    bool Initialize(i32 Argc, const char** Argv);
    void Shutdown();
    bool IsRunning() const;
    void RunFrame();
    void Render(Platform::Window* Window);

    GUI& PushEvent(const Platform::Event& Event);

private:
    struct Buffer
    {
        OctaneGUI::VertexBuffer Data {};
        bool Uploaded { false };
    };

    GUI();

    void OnWindowAction(OctaneGUI::Window* Window, OctaneGUI::WindowAction Action);
    OctaneGUI::Event OnEvent(OctaneGUI::Window* Window);
    Platform::Event PopEvent(Platform::Window* Window);

    UniquePtr<OctaneGUI::Application> m_Application { nullptr };
    HashMap<OctaneGUI::Window*, Platform::Window*> m_Windows {};
    Array<Platform::Event> m_Events {};
    Buffer m_LastBuffer {};
    u32 m_GUIPipeline { 0 };
    u32 m_GUIBuffer { 0 };
    u32 m_WhiteTexture { 0 };
};

}
}
