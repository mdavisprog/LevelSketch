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

#include "DepthStencil.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "Device.hpp"

#import <Metal/Metal.h>

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

DepthStencil::DepthStencil()
{
}

bool DepthStencil::Initialize(Device const* Device_)
{
    @autoreleasepool
    {
        MTLDepthStencilDescriptor* Descriptor = [MTLDepthStencilDescriptor new];
        Descriptor.depthCompareFunction = MTLCompareFunctionLess;
        Descriptor.depthWriteEnabled = YES;

        m_State = [Device_->Get() newDepthStencilStateWithDescriptor:Descriptor];

        if (m_State == nullptr)
        {
            Core::Console::Error("Failed to create depth stencil state.");
            return false;
        }
    }

    return true;
}

bool DepthStencil::UpdateTexture(Device const* Device_, const Vector2i& Size)
{
    bool ShouldUpdate { m_Texture == nullptr };

    if (m_Texture != nullptr)
    {
        ShouldUpdate = Size.X != static_cast<i32>(m_Texture.width) || Size.Y != static_cast<i32>(m_Texture.height);
    }

    if (!ShouldUpdate)
    {
        return false;
    }

    @autoreleasepool
    {
        MTLTextureDescriptor* Descriptor = [MTLTextureDescriptor new];
        Descriptor.width = Size.X;
        Descriptor.height = Size.Y;
        Descriptor.pixelFormat = MTLPixelFormatDepth32Float;
        Descriptor.storageMode = MTLStorageModePrivate;
        Descriptor.usage = MTLTextureUsageRenderTarget;

        m_Texture = [Device_->Get() newTextureWithDescriptor:Descriptor];

        if (m_Texture == nullptr)
        {
            return false;
        }
    }

    return true;
}

id<MTLDepthStencilState> DepthStencil::Get() const
{
    return m_State;
}

id<MTLTexture> DepthStencil::Texture() const
{
    return m_Texture;
}

}
}
}
