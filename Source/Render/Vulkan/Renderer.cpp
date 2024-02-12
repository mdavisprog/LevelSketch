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

#include "Renderer.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Vertex.hpp"
#include "../../Core/Version.hpp"
#include "../../Platform/FileSystem.hpp"
#include "../../Platform/Window.hpp"
#include "../VertexBufferDescription.hpp"
#include "../VertexDataDescription.hpp"
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "DescriptorPool.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "Loader.hpp"
#include "Sync.hpp"
#include "UniformBuffer.hpp"
#include "VertexBuffer.hpp"
#include "Viewport.hpp"

#if defined(DEBUG)
#include "DebugUtils.hpp"
#endif

namespace LevelSketch
{
namespace Render
{

String Renderer::ShadersDirectory()
{
    return Platform::FileSystem::CombinePaths(Platform::FileSystem::ContentDirectory(), "GLSL");
}

namespace Vulkan
{

// FIXME: Better orginazation can be used here.
#if defined(DEBUG)
static const Array<const char*> ValidationLayers { "VK_LAYER_KHRONOS_validation" };
#endif

Renderer::Renderer()
    : LevelSketch::Render::Renderer()
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize()
{
    if (!Loader::Instance().Initialize())
    {
        return false;
    }

    const u32 Version { VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION) };
    VkApplicationInfo AppInfo {};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = APP_NAME;
    AppInfo.applicationVersion = Version;
    AppInfo.pEngineName = APP_NAME;
    AppInfo.engineVersion = Version;
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;
    CreateInfo.enabledLayerCount = 0;

    Array<VkExtensionProperties> ExtProperties;
    if (!Loader::Instance().GetInstanceExtensionProperties(ExtProperties))
    {
        return false;
    }

    Array<const char*> ExtPtrs;
    if (!GetRequiredExtensionProperties(ExtProperties, ExtPtrs))
    {
        return false;
    }

#if defined(DEBUG)
    Array<const char*> LayerPtrs;
    if (GetExistingLayers(ValidationLayers, LayerPtrs))
    {
        Core::Console::WriteLine("Enabling validation layers");
        CreateInfo.enabledLayerCount = static_cast<u32>(LayerPtrs.Size());
        CreateInfo.ppEnabledLayerNames = LayerPtrs.Data();

        ExtPtrs.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo { DebugUtils::CreateInfo() };
    CreateInfo.pNext = &DebugUtilsMessengerCreateInfo;
#endif

    CreateInfo.enabledExtensionCount = static_cast<u32>(ExtPtrs.Size());
    CreateInfo.ppEnabledExtensionNames = ExtPtrs.Data();

    VkResult Result { vkCreateInstance(&CreateInfo, nullptr, &m_Instance) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create instance. Error: %s", Errors::ToString(Result));
        return false;
    }

#if defined(DEBUG)
    DebugUtils::Instance().Initialize(m_Instance);
#endif

    return true;
}

bool Renderer::Initialize(Platform::Window* Window)
{
    UniquePtr<Viewport> Viewport_ { UniquePtr<Viewport>::New() };

    if (!Viewport_->Initialize(m_Instance, Window))
    {
        return false;
    }

    m_Viewports.Push(std::move(Viewport_));

    if (m_Device == nullptr)
    {
        Array<const char*> LayerPtrs;
#if defined(DEBUG)
        GetExistingLayers(ValidationLayers, LayerPtrs);
#endif

        m_Device = UniquePtr<Device>::New();

        if (!m_Device->Initialize(m_Instance, m_Viewports[0]->GetSurface(), LayerPtrs))
        {
            return false;
        }

        m_CommandPool = UniquePtr<CommandPool>::New();

        if (!m_CommandPool->Initialize(m_Device.Get()))
        {
            return false;
        }

        if (!m_CommandPool->InitializeBuffers(m_Device.Get(), FRAMES_IN_FLIGHT))
        {
            return false;
        }

        m_DescriptorPool = UniquePtr<DescriptorPool>::New();

        if (!m_DescriptorPool->Initialize(m_Device.Get(), FRAMES_IN_FLIGHT))
        {
            return false;
        }

        for (i32 I = 0; I < FRAMES_IN_FLIGHT; I++)
        {
            UniquePtr<Sync> Sync_ { UniquePtr<Sync>::New() };

            if (!Sync_->Initialize(m_Device.Get()))
            {
                return false;
            }

            m_Syncs.Push(std::move(Sync_));

            UniquePtr<UniformBuffer> Uniform { UniquePtr<UniformBuffer>::New() };

            if (!Uniform->Initialize(m_Device.Get()))
            {
                return false;
            }

            m_Uniforms.Push(std::move(Uniform));
        }

        Core::Console::WriteLine("Initialized Vulkan");
    }

    m_Viewports.Back()->InitializeSwapChain(m_Device.Get());

    return true;
}

void Renderer::Shutdown()
{
    m_Device->WaitForIdle();

    for (const UniquePtr<VertexBuffer>& Buffer : m_VertexBuffers)
    {
        Buffer->Shutdown(m_Device.Get());
    }

    for (const UniquePtr<UniformBuffer>& Uniform : m_Uniforms)
    {
        Uniform->Shutdown(m_Device.Get());
    }
    m_Uniforms.Clear();

    for (const UniquePtr<Sync>& Sync_ : m_Syncs)
    {
        Sync_->Shutdown(m_Device.Get());
    }
    m_Syncs.Clear();

    m_DescriptorPool->Shutdown(m_Device.Get());
    m_CommandPool->Shutdown(m_Device.Get());

    for (const UniquePtr<Viewport>& Viewport_ : m_Viewports)
    {
        Viewport_->Shutdown(m_Instance, m_Device.Get());
    }
    m_Viewports.Clear();

    m_Device->Shutdown();

#if defined(DEBUG)
    DebugUtils::Instance().Shutdown(m_Instance);
#endif
    /*
        m_RenderBuffer.Shutdown(m_Device);

        m_Pipeline.Shutdown(m_Device);
    */
    if (m_Instance != nullptr)
    {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }

    Loader::Instance().Shutdown();
}
/*
void Renderer::Render(Platform::Window*)
{
    UniformBuffer& Uniforms { m_Uniforms[m_FrameIndex] };
    Uniforms.UpdateBuffer();

    // TODO: Bind uniform buffers
    for each uniform buffer
        Pipeline::BindUniformBuffer

    const Sync& CurrentSync { m_Syncs[m_FrameIndex] };

    CurrentSync.WaitForFence(m_Device);
    const u32 SwapChainFrameIndex { CurrentSync.FrameIndex(m_Device, m_SwapChain) };

    const CommandBuffer& CmdBuffer { m_CommandPool.Buffer(m_FrameIndex) };

    CmdBuffer.Reset();
    CmdBuffer.BeginRecord(m_Pipeline, m_SwapChain, SwapChainFrameIndex);
    CmdBuffer.BindBuffers(m_RenderBuffer);
    CmdBuffer.BindDescriptorSet(m_Pipeline, m_FrameIndex);
    CmdBuffer.DrawVerticesIndexed(6, 1, 0, 0, 0);
    CmdBuffer.EndRecord();
    CmdBuffer.Submit(m_Device, CurrentSync);

    m_SwapChain.Present(m_Device, CurrentSync, SwapChainFrameIndex);

    m_FrameIndex = (m_FrameIndex + 1) % FRAMES_IN_FLIGHT;
}
*/
u32 Renderer::LoadTexture(const void*, u32, u32, u8)
{
    return 1;
}

bool Renderer::BindTexture(u32)
{
    return false;
}

bool Renderer::BeginRender(Platform::Window*, const Colorf&)
{
    return false;
}

void Renderer::EndRender(Platform::Window*)
{
}

void Renderer::SetViewportRect(const ViewportRect& Rect)
{
    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->SetViewport(Rect);
}

void Renderer::SetScissor(const Recti& Rect)
{
    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->SetScissor(Rect);
}

u32 Renderer::CreateGraphicsPipeline(const GraphicsPipelineDescription&)
{
    return 0;
}

bool Renderer::BindGraphicsPipeline(u32)
{
    return false;
}

void Renderer::DrawIndexed(u32, u32, u32, u32, u32)
{
}

u32 Renderer::CreateVertexBuffer(const VertexBufferDescription& Description)
{
    UniquePtr<VertexBuffer> Buffer { UniquePtr<VertexBuffer>::New() };

    if (!Buffer->Initialize(m_Device.Get(), Description))
    {
        return 0;
    }

    const u32 Result { Buffer->ID() };
    m_VertexBuffers.Push(std::move(Buffer));
    return Result;
}

bool Renderer::UploadVertexData(u32 ID, const VertexDataDescription& Description)
{
    VertexBuffer const* Buffer { GetVertexBuffer(ID) };

    if (Buffer == nullptr)
    {
        return false;
    }

    if (!Buffer->GetVertexBuffer()->Upload(m_Device.Get(),
            m_CommandPool.Get(),
            Description.VertexData,
            Description.VertexDataSize))
    {
        return false;
    }

    if (!Buffer->GetIndexBuffer()->Upload(m_Device.Get(),
            m_CommandPool.Get(),
            Description.IndexData,
            Description.IndexDataSize))
    {
        return false;
    }

    return true;
}

bool Renderer::BindVertexBuffer(u32 ID)
{
    VertexBuffer const* Buffer { GetVertexBuffer(ID) };
    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->BindBuffer(Buffer);
    return false;
}

bool Renderer::GetRequiredExtensionProperties(const Array<VkExtensionProperties>& Properties,
    Array<const char*>& Ptrs) const
{
    bool FoundSurface { false };

    for (const VkExtensionProperties& Property : Properties)
    {
        const String Name { Property.extensionName };
        if (Name == "VK_KHR_surface" || Name == "VK_KHR_xlib_surface" || Name == "VK_KHR_xcb_surface" ||
            Name == "VK_KHR_wayland_surface")
        {
            FoundSurface |= Name == "VK_KHR_surface";
            Ptrs.Push(Property.extensionName);
        }
    }

    if (!FoundSurface)
    {
        Core::Console::Error("Failed to load window surface extensions.");
        return false;
    }

    return true;
}

bool Renderer::GetExistingLayers(const Array<const char*> Layers, Array<const char*>& Ptrs) const
{
    Ptrs.Clear();

    u32 Count { 0 };
    VkResult Result { vkEnumerateInstanceLayerProperties(&Count, nullptr) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::WriteLine("Failed to retrieve layer properties count. Error: %s", Errors::ToString(Result));
        return false;
    }

    Array<VkLayerProperties> Properties;
    Properties.Resize(Count);
    Result = vkEnumerateInstanceLayerProperties(&Count, Properties.Data());
    if (Result != VK_SUCCESS)
    {
        Core::Console::WriteLine("Failed to retrieve layer properties. Error: %s", Errors::ToString(Result));
        return false;
    }

    for (const char* Name : Layers)
    {
        for (const VkLayerProperties& Layer : Properties)
        {
            if (String(Name) == Layer.layerName)
            {
                Ptrs.Push(Name);
            }
        }
    }

    return !Ptrs.IsEmpty();
}

VertexBuffer const* Renderer::GetVertexBuffer(u32 ID) const
{
    for (const UniquePtr<VertexBuffer>& Buffer : m_VertexBuffers)
    {
        if (Buffer->ID() == ID)
        {
            return Buffer.Get();
        }
    }

    return nullptr;
}

}
}
}
