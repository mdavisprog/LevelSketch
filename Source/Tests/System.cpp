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

#include "System.hpp"
#include "../Core/Version.hpp"
#include "Core/Core.hpp"
#include "TestSuite.hpp"

#include <cstdio>

namespace LevelSketch
{
namespace Tests
{

System& System::Instance()
{
    static System Instance {};
    return Instance;
}

i32 System::Run(i32, char**)
{
    printf("\nRunning %s testing framework version %d.%d.%d.\n", APP_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
    printf("There are (%llu) test suites to run through.\n\n", m_TestSuites.Size());

    for (TestSuite* Suite : m_TestSuites)
    {
        Suite->Run();
        printf("\n");
    }

    Shutdown();

    printf("Finished running tests.\n\n");
    return 0;
}

System::System()
{
    m_TestSuites.Push(Core::Array());
}

System& System::Shutdown()
{
    for (const TestSuite* Suite : m_TestSuites)
    {
        delete Suite;
    }

    m_TestSuites.Clear();

    return *this;
}

}
}
