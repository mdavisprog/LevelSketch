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

#include "../../Core/Memory/Shareable.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"
#include "Core.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

class ShareableObject : private Shareable<ShareableObject>
{
public:
    ShareableObject()
    {
    }

    SharedPtr<ShareableObject> Clone() const
    {
        return Share();
    }
};

static bool ShareNull()
{
    ShareableObject Instance;
    SharedPtr<ShareableObject> SharedPtr { Instance.Clone() };
    VERIFY(SharedPtr.IsNull());
    return true;
}

static bool ShareInstance()
{
    SharedPtr<ShareableObject> Instance { SharedPtr<ShareableObject>::New() };
    VERIFY(Instance.GetReferenceCount() == 1);
    SharedPtr<ShareableObject> Copy { Instance->Clone() };
    VERIFY(!Copy.IsNull());
    VERIFY(Instance.GetReferenceCount() == 2);
    return true;
}

UniquePtr<TestSuite> ShareableTests()
{
    return TestSuite::New("Shareable", { TEST_CASE(ShareNull), TEST_CASE(ShareInstance) });
}

}
}
}
