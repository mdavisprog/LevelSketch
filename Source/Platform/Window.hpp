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

#include "../Core/Math/Forwards.hpp"
#include "../Core/Types.hpp"

namespace LevelSketch
{

namespace Platform
{

struct WindowDescription;

class Window
{
public:
    virtual ~Window()
    {
    }

    virtual void* Handle() const = 0;
    virtual bool Create(const WindowDescription& Description) = 0;
    virtual void Close() = 0;
    virtual void Show() = 0;
    virtual void Focus() = 0;
    virtual void SetPosition(i32 X, i32 Y) = 0;
    virtual Vector2i Position() const = 0;
    virtual Vector2i Size() const = 0;
    virtual void ProcessEvents() = 0;
    virtual Vector2 ContentScale() const;

    virtual bool IsOpen() const = 0;

    f32 AspectRatio() const;
    Recti Bounds() const;
};

}
}
