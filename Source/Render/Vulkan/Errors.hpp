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

#include "vulkan/vulkan.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{
namespace Errors
{

static const char* ToString(VkResult Error)
{
    switch (Error)
    {
    case VK_INCOMPLETE: return "Incomplete";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "Out of host memory";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "Out of device memory";
    case VK_ERROR_LAYER_NOT_PRESENT: return "Layer not present";
    case VK_ERROR_INITIALIZATION_FAILED: return "Initialization failed";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "Extension not present";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "Incompatible driver";
    default: break;
    }

    return "Success";
}

}
}
}
}