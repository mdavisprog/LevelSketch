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

#include "../Core/Types.hpp"
#include "../Core/Math/Vector2.hpp"

namespace LevelSketch
{
namespace Platform
{

class Window;

struct Event
{
    friend class EventQueue;

public:

    //
    // Enum
    //

    enum class Type : u8
    {
        None,
        MouseMove,
    };

    //
    // Structs
    //

    struct OnMouseMove
    {
        Vector2i Position {};
    };

    //
    // Union
    //

    union Data
    {
        OnMouseMove MouseMove;
    };

    Event()
    {
    }

    Event(const OnMouseMove& Data)
        : m_Type(Type::MouseMove)
    {
        m_Data.MouseMove = Data;
    }

    Type GetType() const
    {
        return m_Type;
    }

    const Data& GetData() const
    {
        return m_Data;
    }

    Window* GetWindow() const
    {
        return m_Window;
    }

private:
    Type m_Type { Type::None };
    Data m_Data { 0 };
    Window* m_Window { nullptr };
};


}
}