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

#include "Platform.hpp"
#include "../../Platform/FileSystem.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{

namespace Tests
{
namespace Platform
{

using FileSystem = LevelSketch::Platform::FileSystem;

static bool GetDirectory()
{
    String Path { "File" };
    VERIFY(FileSystem::GetDirectory(Path) == "File");
    Path = "/User/Test/File";
    VERIFY(FileSystem::GetDirectory(Path) == "/User/Test");
    Path = "C:\\User\\Test\\File";
    VERIFY(FileSystem::GetDirectory(Path) == "C:\\User\\Test");
    Path = "/User/Test/";
    VERIFY(FileSystem::GetDirectory(Path) == "/User/Test");
    Path = "/User\\Test\\File";
    VERIFY(FileSystem::GetDirectory(Path) == "/User\\Test");
    Path = "C:\\User/Test/File";
    VERIFY(FileSystem::GetDirectory(Path) == "C:\\User/Test");
    return true;
}

UniquePtr<TestSuite> FileSystemTests()
{
    return TestSuite::New("FileSystem", {
        TEST_CASE(GetDirectory)
    });
}

}
}
}
