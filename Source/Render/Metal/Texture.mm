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

#include "Texture.hpp"
#include "../../Core/Console.hpp"
#include "Device.hpp"

#import <MetalKit/MetalKit.h>

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

u32 Texture::s_ID { 0 };

Texture::Texture()
{
}

bool Texture::Initialize(Device const* Device_, u32 Width, u32 Height)
{
    MTLTextureDescriptor* TextureDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                           width:Width
                                                          height:Height
                                                       mipmapped:NO];

    TextureDesc.usage = MTLTextureUsageShaderRead;
    TextureDesc.storageMode = MTLStorageModeManaged;

    m_Texture = [Device_->Get() newTextureWithDescriptor:TextureDesc];

    if (m_Texture == nullptr)
    {
        Core::Console::Error("Failed to create texture.");
        return false;
    }

    m_ID = ++s_ID;

    return true;
}

bool Texture::Upload(const void* Data, u64 BytesPerRow)
{
    MTLRegion Region = MTLRegionMake2D(0, 0, m_Texture.width, m_Texture.height);
    [m_Texture replaceRegion:Region mipmapLevel:0 withBytes:Data bytesPerRow:BytesPerRow];

    return true;
}

u32 Texture::ID() const
{
    return m_ID;
}

id<MTLTexture> Texture::Data() const
{
    return m_Texture;
}

}
}
}
