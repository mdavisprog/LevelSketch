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

#include "../../Platform/FileSystem.hpp"
#include "../../Core/Containers/String.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"
#include "Platform.hpp"

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

static bool GetFileName()
{
    VERIFY(FileSystem::GetFileName("C:\\Hello.txt") == "Hello.txt");
    VERIFY(FileSystem::GetFileName("/User/Hello.txt") == "Hello.txt");
    VERIFY(FileSystem::GetFileName("C:\\Hello\\World") == "World");
    VERIFY(FileSystem::GetFileName("/User/Hello/World") == "World");
    VERIFY(FileSystem::GetFileName("C:\\Hello\\World\\") == "");
    VERIFY(FileSystem::GetFileName("/User/Hello/World/") == "");
    VERIFY(FileSystem::GetFileName("Hello.txt") == "Hello.txt");
    return true;
}

static bool GetBaseFileName()
{
    VERIFY(FileSystem::GetBaseFileName("Hello.txt") == "Hello");
    VERIFY(FileSystem::GetBaseFileName("C:\\Hello.txt") == "Hello");
    VERIFY(FileSystem::GetBaseFileName("/User/Hello.txt") == "Hello");
    VERIFY(FileSystem::GetBaseFileName("Hello") == "Hello");
    return true;
}

static bool SetExtension()
{
    VERIFY(FileSystem::SetExtension("Hello", "txt") == "Hello.txt");
    VERIFY(FileSystem::SetExtension("Hello.txt", "md") == "Hello.md");
    VERIFY(FileSystem::SetExtension("C:\\Hello.txt", "md") == "C:\\Hello.md");
    VERIFY(FileSystem::SetExtension("/User/Hello.txt", "md") == "/User/Hello.md");
    VERIFY(FileSystem::SetExtension("C:\\Hello\\", "git") == "C:\\Hello\\.git");
    return true;
}

UniquePtr<TestSuite> FileSystemTests()
{
    return TestSuite::New("FileSystem",
        { TEST_CASE(GetDirectory), TEST_CASE(GetFileName), TEST_CASE(GetBaseFileName), TEST_CASE(SetExtension) });
}

}
}
}
