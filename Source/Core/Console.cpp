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

#include "Console.hpp"
#include "Compiler.hpp"

#include <climits>

namespace LevelSketch
{
namespace Core
{

Console& Console::Instance()
{
    static Console Instance {};
    return Instance;
}

Console& Console::Write(const char* Format, ...)
{
    va_list List;
    va_start(List, Format);

    Instance().Write(Format, List);

    va_end(List);

    return Instance();
}

Console& Console::WriteLine(const char* Format, ...)
{
    va_list List;
    va_start(List, Format);

    Instance().Write(Format, List);
    printf("\n");

    va_end(List);

    return Instance();
}

Console& Console::PushForegroundColor(Color Foreground)
{
    Instance().m_ForegroundColorStack.Push(Foreground);
    return Instance();
}

Console& Console::PopForegroundColor()
{
    Instance().m_ForegroundColorStack.Pop();
    return Instance();
}

Console::Console()
{
    m_Buffer.Reserve(SHRT_MAX);
}

static const char* AsciiEscape(Console::Color Color, bool Foreground)
{
    switch (Color)
    {
    case Console::Color::Black: return Foreground ? "30" : "40";
    case Console::Color::Red: return Foreground ? "31" : "41";
    case Console::Color::Green: return Foreground ? "32" : "42";
    case Console::Color::Yellow: return Foreground ? "33" : "43";
    case Console::Color::Blue: return Foreground ? "34" : "44";
    case Console::Color::Magenta: return Foreground ? "35" : "45";
    case Console::Color::Cyan: return Foreground ? "36" : "46";
    case Console::Color::White: return Foreground ? "37" : "47";
    case Console::Color::Default: return Foreground ? "39" : "49";
    case Console::Color::Reset: break;
    }

    return "0";
}

Console& Console::Write(const char* Format, const va_list& List)
{
#if defined(MSVC)
    vsnprintf_s(m_Buffer.Data(), m_Buffer.Capacity(), _TRUNCATE, Format, List);
#else
    vsprintf(m_Buffer.Data(), m_Buffer.Capacity(), Format, List);
#endif

    if (!m_ForegroundColorStack.IsEmpty())
    {
        printf("\33[%sm", AsciiEscape(m_ForegroundColorStack.Back(), true));
    }

    printf("%s", m_Buffer.Data());

    if (!m_ForegroundColorStack.IsEmpty())
    {
        m_ForegroundColorStack.Pop();
        printf("\33[0m");
    }

    return *this;
}

}
}
