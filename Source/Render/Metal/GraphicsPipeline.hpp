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

#include "../../Core/Types.hpp"
#include "../Handle.hpp"

#import <Metal/Metal.h>

namespace LevelSketch
{
namespace Render
{

struct GraphicsPipelineDescription;

namespace Metal
{

class Device;

class GraphicsPipeline
{
public:
    GraphicsPipeline();

    bool Initialize(Device const* Device_, const GraphicsPipelineDescription& Description);
    id<MTLRenderPipelineState> Get() const;
    id<MTLDepthStencilState> DepthStencil() const;
    GraphicsPipelineHandle Handle() const;
    MTLCullMode CullMode() const;

private:
    id<MTLRenderPipelineState> m_State { nullptr };
    id<MTLDepthStencilState> m_DepthStencil { nullptr };
    GraphicsPipelineHandle m_Handle {};
    MTLCullMode m_CullMode { MTLCullModeNone };
};

}
}
}
