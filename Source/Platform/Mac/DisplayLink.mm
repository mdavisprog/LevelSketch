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

#include "DisplayLink.hpp"
#include "../../Core/Console.hpp"
#include "Platform.hpp"
// FIXME: This is needed due to forward declaration of class in Platform.hpp.
#include "Window.hpp"

namespace LevelSketch
{
namespace Platform
{
namespace Mac
{

static CVReturn OnDisplayLinkOutput(CVDisplayLinkRef, // DisplayLink,
    const CVTimeStamp*,                               // Now,
    const CVTimeStamp* OutputTime,
    CVOptionFlags,  // FlagsIn,
    CVOptionFlags*, // FlagsOut,
    void* DisplayLinkContext)
{
    Mac::Platform* Platform { static_cast<Mac::Platform*>(Platform::Platform::Instance().Get()) };
    Platform->UpdateVideoTime(OutputTime->videoTime, OutputTime->videoTimeScale);

    __weak dispatch_source_t Source { (__bridge dispatch_source_t)DisplayLinkContext };
    dispatch_source_merge_data(Source, 1);

    return kCVReturnSuccess;
}

DisplayLink& DisplayLink::Instance()
{
    static DisplayLink Instance {};
    return Instance;
}

DisplayLink::~DisplayLink()
{
}

bool DisplayLink::Initialize()
{
    m_DisplaySource = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_ADD, 0, 0, dispatch_get_main_queue());
    dispatch_source_set_event_handler(m_DisplaySource, ^() {
        @autoreleasepool
        {
            Platform::Platform::Instance()->RunFrame();
        }
    });
    dispatch_resume(m_DisplaySource);

    CVReturn Return = CVDisplayLinkCreateWithActiveCGDisplays(&m_DisplayLink);
    if (Return != kCVReturnSuccess)
    {
        LevelSketch::Core::Console::Error("Failed to create display link.");
        return false;
    }

    Return = CVDisplayLinkSetOutputCallback(m_DisplayLink, &OnDisplayLinkOutput, (__bridge void*)m_DisplaySource);

    if (Return != kCVReturnSuccess)
    {
        LevelSketch::Core::Console::Error("Failed to display link output callback.");
        return false;
    }

    CVDisplayLinkStart(m_DisplayLink);

    return true;
}

void DisplayLink::Shutdown()
{
    CVDisplayLinkStop(m_DisplayLink);
    dispatch_source_cancel(m_DisplaySource);
}

bool DisplayLink::Initialized() const
{
    return m_DisplayLink != nullptr;
}

DisplayLink::DisplayLink()
{
}

}
}
}
