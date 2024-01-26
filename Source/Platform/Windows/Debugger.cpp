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

#include "Debugger.hpp"
#include "../../Core/CommandLine.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Defines.hpp"
#include "Common.hpp"

#if defined(DEBUG)
    #define _CRTDBG_MAP_ALLOC
    #include <cstdlib>
    #include <crtdbg.h>
#endif

namespace LevelSketch
{
namespace Platform
{
namespace Windows
{

#if defined(DEBUG)
    static bool HasLeakCheack()
    {
        return Core::CommandLine::Instance().Has("--leak-check");
    }

    static void StartMemCapture()
    {
        if (!HasLeakCheack())
        {
            return;
        }

        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        Core::Console::WriteLine("Enabling memory leak check. Output is only dumped through Visual Studio at the moment.");
    }
#endif

Debugger::Debugger()
{
}

bool Debugger::Initialize()
{
#if defined(DEBUG)
    StartMemCapture();
#endif

    return LevelSketch::Platform::Debugger::Initialize();
}

}
}
}
