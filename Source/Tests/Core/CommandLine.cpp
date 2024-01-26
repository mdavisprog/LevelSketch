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

#include "Core.hpp"
#include "../../Core/CommandLine.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

using CommandLine = LevelSketch::Core::CommandLine;

class CommandLineState
{
public:
    CommandLineState()
    {
        m_Arguments = CommandLine::Instance().Arguments();
    }

    ~CommandLineState()
    {
        CommandLine::Instance().Set(m_Arguments);
    }

private:
    Array<String> m_Arguments {};
};

static bool Set()
{
    CommandLineState State {};
    const char* Args[] = { "Hello", "World" };
    CommandLine::Instance().Set(ARRAY_COUNT(Args), Args);
    VERIFY(CommandLine::Instance().Count() == 2);
    VERIFY(CommandLine::Instance().Get(0) == "Hello");
    VERIFY(CommandLine::Instance().Get(1) == "World");
    return true;
}

static bool Has()
{
    CommandLineState State {};
    const char* Args[] = { "Hello", "World" };
    CommandLine::Instance().Set(ARRAY_COUNT(Args), Args);
    VERIFY(CommandLine::Instance().Has("World"));
    VERIFY(!CommandLine::Instance().Has("Foo"));
    return true;
}

UniquePtr<TestSuite> CommandLineTests()
{
    return TestSuite::New("CommandLine", {
        TEST_CASE(Set),
        TEST_CASE(Has)
    });
}

}
}
}
