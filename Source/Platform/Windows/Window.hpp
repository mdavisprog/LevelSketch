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

#include "../Window.hpp"
#include "Common.hpp"

namespace LevelSketch
{
namespace Platform
{
namespace Windows
{

class Window : public LevelSketch::Platform::Window
{
public:
    Window();

    virtual void* Handle() const override;
    virtual bool Create(const char* Title, int X, int Y, int Width, int Height) override;
    virtual void Close() override;
    virtual void Show() override;
    virtual void Focus() override;
    virtual void SetPosition(int X, int Y) override;
    virtual Core::Math::Vector2i Position() const override;
    virtual Core::Math::Vector2i Size() const override;
    virtual void ProcessEvents() override;

    virtual bool IsOpen() const override;

private:
    HWND m_Handle { nullptr };
};

}
}
}
