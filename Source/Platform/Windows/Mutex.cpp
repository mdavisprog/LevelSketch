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

#include "Mutex.hpp"
#include "../../Core/Console.hpp"

namespace LevelSketch
{
namespace Platform
{

UniquePtr<Mutex> Mutex::New()
{
    UniquePtr<Mutex> Result { UniquePtr<Windows::Mutex>::New() };

    if (!Result->Create())
    {
        return nullptr;
    }

    return Result;
}

namespace Windows
{

Mutex::Mutex()
    : LevelSketch::Platform::Mutex()
{
}

bool Mutex::Create()
{
    if (IsValid())
    {
        return true;
    }

    m_Handle = CreateMutexW(nullptr, FALSE, nullptr);
    if (m_Handle == nullptr)
    {
        Core::Console::Error("Failed to create mutex with error code: 0x%08X", GetLastError());
    }

    return m_Handle != nullptr;
}

void Mutex::Destroy()
{
    if (!IsValid())
    {
        return;
    }

    CloseHandle(m_Handle);
    m_Handle = nullptr;
}

bool Mutex::IsValid() const
{
    return m_Handle != nullptr;
}

void Mutex::Lock() const
{
    if (!IsValid())
    {
        return;
    }

    WaitForSingleObject(m_Handle, INFINITE);
}

void Mutex::Unlock() const
{
    if (!IsValid())
    {
        return;
    }

    ReleaseMutex(m_Handle);
}

}
}
}
