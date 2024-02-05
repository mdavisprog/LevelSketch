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

#pragma once

#include "../Core/Containers/Forwards.hpp"
#include "../Core/Types.hpp"

namespace LevelSketch
{

namespace Platform
{

// TODO: Look into creating a Path class to manage file paths.

class FileSystem final
{
public:
    static String ApplicationPath();
    static String ApplicationDirectory();
    static String ContentDirectory();
    static String ShadersDirectory();
    static String GetDirectory(const String& Path);
    static String CombinePaths(const String& A, const String& B);
    static String ReadContents(const String& Path);
    static Array<u8> ReadBinaryContents(const String& Path);

    static void SetWorkingDirectory(const String& Path);

private:
    FileSystem();
};

}
}
