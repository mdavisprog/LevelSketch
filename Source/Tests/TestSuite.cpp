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

#include "TestSuite.hpp"
#include "../Core/Console.hpp"
#include "../Core/Memory/UniquePtr.hpp"

#include <cstdio>

namespace LevelSketch
{
namespace Tests
{

UniquePtr<TestSuite> TestSuite::New(const char* Name, Array<TestSuite::TestCase>&& TestCases)
{
    return UniquePtr<TestSuite>::New(Name, std::move(TestCases));
}

TestSuite::TestSuite(const char* Name, Array<TestCase>&& TestCases)
    : m_Name(Name)
    , m_TestCases(std::move(TestCases))
{
}

bool TestSuite::Run()
{
    u64 Succeeded { 0 };
    for (const TestCase& Case : m_TestCases)
    {
        const bool Result { Case.OnTestCase() };

        if (Result)
        {
            Succeeded++;
        }
        else
        {
            Core::Console::WriteLine(Core::Console::Color::Red, "'%s' test case has failed.", Case.Name.data());
        }
    }

    const bool Result { Succeeded == NumTestCases() };
    const Core::Console::Color Color { Result ? Core::Console::Color::Green : Core::Console::Color::Red };
    Core::Console::WriteLine(Color, "'%s' was completed with %llu/%llu test cases passed.", Name(), Succeeded, m_TestCases.Size());

    return Result;
}

const char* TestSuite::Name() const
{
    return m_Name.data();
}

u64 TestSuite::NumTestCases() const
{
    return m_TestCases.Size();
}

}
}
