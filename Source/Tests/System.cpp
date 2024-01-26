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

#include "System.hpp"
#include "../Core/Console.hpp"
#include "../Core/Version.hpp"
#include "../Platform/Debugger.hpp"
#include "Core/Core.hpp"
#include "Engine/Engine.hpp"
#include "Platform/Platform.hpp"
#include "TestSuite.hpp"

#include <cstdio>

namespace LevelSketch
{
namespace Tests
{

using Console = LevelSketch::Core::Console;

System& System::Instance()
{
    static System Instance {};
    return Instance;
}

i32 System::Run()
{
    Console::WriteLine("\nRunning %s testing framework version %d.%d.%d.", APP_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
    Console::WriteLine("There are (%llu) test suites to run through.\n", m_TestSuites.Size());

    bool Success { true };
    for (UniquePtr<TestSuite>& Suite : m_TestSuites)
    {
        Success &= Suite->Run();
    }
    Console::WriteLine("");

    Shutdown();

    if (Success)
    {
        Console::WriteLine("Finished running tests.\n");
    }
    else
    {
        Console::WriteLine(Console::Color::Red, "An error has occurred during testing.\n");
    }

    LevelSketch::Platform::Debugger::Instance()->Shutdown();
    return 0;
}

System::System()
{
    m_TestSuites
        .Push(Core::ArrayTests())
        .Push(Core::CommandLineTests())
        .Push(Core::ListTests())
        .Push(Core::HashMapTests())
        .Push(Core::MathTests())
        .Push(Core::RedBlackTreeTests())
        .Push(Core::MapTests())
        .Push(Core::OptionalTests())
        .Push(Core::ShareableTests())
        .Push(Core::SharedPtrTests())
        .Push(Core::StringTests())
        .Push(Core::UniquePtrTests())
        .Push(Core::WeakPtrTests())

        .Push(Engine::ClassTests())
        .Push(Engine::TypeDatabaseTests())

        .Push(Platform::FileSystemTests());
}

System& System::Shutdown()
{
    m_TestSuites.Clear();
    return *this;
}

}
}
