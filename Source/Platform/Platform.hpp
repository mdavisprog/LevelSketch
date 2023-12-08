/**

MIT License

Copyright (c) 2023 Mitchell Davis <mdavisprog@gmail.com>

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

#include "../Core/Containers/Array.hpp"
#include "../Core/Memory/UniquePtr.hpp"

namespace LevelSketch
{
namespace Platform
{

class Window;

struct TimingData
{
public:
    f32 DeltaSeconds { 0.0f };
    f32 TotalTimeSeconds { 0.0f };
};

class Platform
{
public:
    typedef bool (*OnFrameSignature)(const TimingData&);

    static const Core::Memory::UniquePtr<Platform>& Instance();

    Platform() {}
    virtual ~Platform() {}

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual const char* Name() const = 0;

    virtual int Run();
    bool RunFrame();

    TimingData GetTimingData() const;

    Platform& SetOnFrame(OnFrameSignature&& Fn);

    Window* NewWindow(const char* Title, i32 X, i32 Y, i32 Width, i32 Height);
    Platform& CloseWindow(Window* Window);
    const Core::Containers::Array<Core::Memory::UniquePtr<Window>>& Windows() const;

protected:
    virtual Core::Memory::UniquePtr<Window> InternalNewWindow() const = 0;
    virtual void UpdateTimingData(TimingData& Data);

private:
    OnFrameSignature m_OnFrame { nullptr };
    Core::Containers::Array<Core::Memory::UniquePtr<Window>> m_Windows {};
    TimingData m_TimingData {};
};

}
}
