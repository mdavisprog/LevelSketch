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

#include "CommandLine.hpp"

namespace LevelSketch
{
namespace Core
{

CommandLine& CommandLine::Instance()
{
    static CommandLine Instance;
    return Instance;
}

CommandLine& CommandLine::Set(i32 Argc, const char** Argv)
{
    m_Arguments.Clear();

    for (i32 I = 0; I < Argc; I++)
    {
        m_Arguments.Push(Argv[I]);
    }

    return *this;
}

CommandLine& CommandLine::Set(const Array<String>& Arguments)
{
    m_Arguments = Arguments;
    return *this;
}

u64 CommandLine::Count() const
{
    return m_Arguments.Size();
}

const Array<String>& CommandLine::Arguments() const
{
    return m_Arguments;
}

String CommandLine::Get(u64 Index) const
{
    return m_Arguments[Index];
}

bool CommandLine::Has(const char* Argument) const
{
    for (const String& Arg : m_Arguments)
    {
        if (Arg == Argument)
        {
            return true;
        }
    }

    return false;
}

CommandLine::CommandLine()
{
}

}
}
