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

#include "../../Core/Math/Forwards.hpp"
#include "../../Core/Memory/UniquePtr.hpp"

@protocol CAMetalDrawable;
@protocol MTLCommandBuffer;
@protocol MTLTexture;

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

class CommandEncoder;
class CommandQueue;

class CommandBuffer final
{
public:
    CommandBuffer();
    ~CommandBuffer();

    bool Initialize(CommandQueue const* Queue);
    id<MTLCommandBuffer> Get() const;

    CommandEncoder* BeginEncoding(const Colorf& ClearColor, id<CAMetalDrawable> Drawable, id<MTLTexture> DepthTexture);
    CommandEncoder* CurrentEncoder() const;
    void EndEncoding();

private:
    id<MTLCommandBuffer> m_CommandBuffer { nullptr };
    id<CAMetalDrawable> m_Drawable { nullptr };
    UniquePtr<CommandEncoder> m_CommandEncoder { nullptr };
};

}
}
}
