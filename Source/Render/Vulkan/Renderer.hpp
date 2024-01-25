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

#include "../Renderer.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Queue.hpp"
#include "RenderBuffer.hpp"
#include "Surface.hpp"
#include "SwapChain.hpp"
#include "Sync.hpp"
#include "UniformBuffer.hpp"
#include "vulkan/vulkan.hpp"

#define FRAMES_IN_FLIGHT 2

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class Renderer : public LevelSketch::Render::Renderer
{
public:
    Renderer();

    virtual bool Initialize() override;
    virtual bool Initialize(Platform::Window* Window) override;
    virtual void Shutdown() override;
    virtual void Render(Platform::Window* Window) override;
    virtual u32 LoadTexture(const void* Data, u32 Width, u32 Height, u8 BytesPerPixel = 4) override;
    virtual void UploadGUIData(OctaneGUI::Window* Window, const OctaneGUI::VertexBuffer& Buffer) override;

private:
    bool GetRequiredExtensionProperties(const Array<VkExtensionProperties>& Properties, Array<const char*>& Ptrs) const;
    bool GetExistingLayers(const Array<const char*> Layers, Array<const char*>& Ptrs) const;

    VkInstance m_Instance { nullptr };
    Surface m_Surface {};
    Device m_Device {};
    SwapChain m_SwapChain {};
    GraphicsPipeline m_Pipeline {};
    Sync m_Syncs[FRAMES_IN_FLIGHT] {};
    CommandPool m_CommandPool {};
    u64 m_FrameIndex { 0 };
    
    RenderBuffer m_RenderBuffer {};
    UniformBuffer m_Uniforms[FRAMES_IN_FLIGHT] {};
};

}
}
}
