
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

#include "EventView.hpp"
#include "../EventQueue.hpp"

#import <Carbon/Carbon.h>

namespace Platform = LevelSketch::Platform;

static LevelSketch::Vector2i TransformPosition(const NSView* View, NSPoint Position)
{
    return { static_cast<i32>(Position.x), static_cast<i32>(View.superview.frame.size.height - Position.y) };
}

static bool IsPressed(NSEventType Type)
{
    return Type == NSEventTypeLeftMouseDown || Type == NSEventTypeRightMouseDown;
}

static Platform::Mouse::Button::Type ToButton(NSEventType Type)
{
    switch (Type)
    {
    case NSEventTypeLeftMouseDown:
    case NSEventTypeLeftMouseUp: return Platform::Mouse::Button::Left;
    case NSEventTypeRightMouseDown:
    case NSEventTypeRightMouseUp: return Platform::Mouse::Button::Right;
    default: break;
    }

    return Platform::Mouse::Button::None;
}

static Platform::Keyboard::Key ToKey(unsigned short KeyCode)
{
    switch (KeyCode)
    {
    case kVK_ANSI_A: return Platform::Keyboard::Key::A;
    case kVK_ANSI_B: return Platform::Keyboard::Key::B;
    case kVK_ANSI_C: return Platform::Keyboard::Key::C;
    case kVK_ANSI_D: return Platform::Keyboard::Key::D;
    case kVK_ANSI_E: return Platform::Keyboard::Key::E;
    case kVK_ANSI_F: return Platform::Keyboard::Key::F;
    case kVK_ANSI_G: return Platform::Keyboard::Key::G;
    case kVK_ANSI_H: return Platform::Keyboard::Key::H;
    case kVK_ANSI_I: return Platform::Keyboard::Key::I;
    case kVK_ANSI_J: return Platform::Keyboard::Key::J;
    case kVK_ANSI_K: return Platform::Keyboard::Key::K;
    case kVK_ANSI_L: return Platform::Keyboard::Key::L;
    case kVK_ANSI_M: return Platform::Keyboard::Key::M;
    case kVK_ANSI_N: return Platform::Keyboard::Key::N;
    case kVK_ANSI_O: return Platform::Keyboard::Key::O;
    case kVK_ANSI_P: return Platform::Keyboard::Key::P;
    case kVK_ANSI_Q: return Platform::Keyboard::Key::Q;
    case kVK_ANSI_R: return Platform::Keyboard::Key::R;
    case kVK_ANSI_S: return Platform::Keyboard::Key::S;
    case kVK_ANSI_T: return Platform::Keyboard::Key::T;
    case kVK_ANSI_U: return Platform::Keyboard::Key::U;
    case kVK_ANSI_V: return Platform::Keyboard::Key::V;
    case kVK_ANSI_W: return Platform::Keyboard::Key::W;
    case kVK_ANSI_X: return Platform::Keyboard::Key::X;
    case kVK_ANSI_Y: return Platform::Keyboard::Key::Y;
    case kVK_ANSI_Z: return Platform::Keyboard::Key::Z;
    case kVK_ANSI_1: return Platform::Keyboard::Key::One;
    case kVK_ANSI_2: return Platform::Keyboard::Key::Two;
    case kVK_ANSI_3: return Platform::Keyboard::Key::Three;
    case kVK_ANSI_4: return Platform::Keyboard::Key::Four;
    case kVK_ANSI_5: return Platform::Keyboard::Key::Five;
    case kVK_ANSI_6: return Platform::Keyboard::Key::Six;
    case kVK_ANSI_7: return Platform::Keyboard::Key::Seven;
    case kVK_ANSI_8: return Platform::Keyboard::Key::Eight;
    case kVK_ANSI_9: return Platform::Keyboard::Key::Nine;
    case kVK_ANSI_0: return Platform::Keyboard::Key::Zero;
    default: break;
    }

    return Platform::Keyboard::Key::None;
}

@implementation EventView

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)viewDidMoveToWindow
{
    [self.window makeFirstResponder:self];
}

- (NSView*)hitTest:(NSPoint)Point
{
    return self;
}

- (void)mouseMoved:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)mouseDragged:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)mouseDown:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)mouseUp:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)rightMouseMoved:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)rightMouseDragged:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)rightMouseDown:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)rightMouseUp:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)keyDown:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)keyUp:(NSEvent*)Event
{
    [self HandleEvent:Event];
}

- (void)HandleEvent:(NSEvent*)Event
{
    switch (Event.type)
    {

    case NSEventTypeMouseMoved:
    case NSEventTypeLeftMouseDragged:
    case NSEventTypeRightMouseDragged:
    {
        Platform::Event::OnMouseMove MouseMove { TransformPosition(self, Event.locationInWindow) };

        Platform::EventQueue::Instance().Push(MouseMove, _Window);
    }
    break;

    case NSEventTypeLeftMouseDown:
    case NSEventTypeLeftMouseUp:
    case NSEventTypeRightMouseDown:
    case NSEventTypeRightMouseUp:
    {
        Platform::Event::OnMouseButton MouseButton {};
        MouseButton.Button = ToButton(Event.type);
        MouseButton.Pressed = IsPressed(Event.type);
        MouseButton.Position = TransformPosition(self, Event.locationInWindow);

        Platform::EventQueue::Instance().Push(MouseButton, _Window);
    }
    break;

    case NSEventTypeKeyDown:
    case NSEventTypeKeyUp:
    {
        const Platform::Event::OnKey Key { ToKey(Event.keyCode), Event.type == NSEventTypeKeyDown };
        Platform::EventQueue::Instance().Push(Key, _Window);
    }
    break;

    default: break;
    }
}

@end
