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

#include "../Core/Containers/Array.hpp"
#include "../Core/Memory/UniquePtr.hpp"
#include "../Core/Types.hpp"

#include <string>

#ifndef STRINGIFY
#define STRINGIFY(X) #X
#endif

// Utility initializer for in-place construction of test cases for a test suite for example:
// return new TestSuite("TestSuite", {
//     TEST_CASE(Test1),
//     TEST_CASE(Test2)
// });
#define TEST_CASE(Fn)                                                                                                  \
    {                                                                                                                  \
        STRINGIFY(Fn), Fn                                                                                              \
    }

namespace LevelSketch
{

namespace Tests
{

class TestSuite final
{
public:
    struct TestCase
    {
    public:
        std::string Name {};
        bool (*OnTestCase)();
    };

    static UniquePtr<TestSuite> New(const char* Name, Array<TestCase>&& TestCases);

    TestSuite(const char* Name, Array<TestCase>&& TestCases);

    bool Run();

    const char* Name() const;
    u64 NumTestCases() const;

private:
    std::string m_Name {};
    Array<TestCase> m_TestCases {};
};

}
}
