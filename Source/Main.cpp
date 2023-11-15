#include <cstdio>
#include "OctaneGUI/OctaneGUI.h"
#include <Windows.h>

#define CLASS_NAME L"LevelSketch"

std::unordered_map<OctaneGUI::Window*, HWND> g_Windows {};

LRESULT WinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    case WM_CLOSE:
        for (std::unordered_map<OctaneGUI::Window*, HWND>::const_iterator It { g_Windows.begin() }; It != g_Windows.end(); ++It)
        {
            if (It->second == hWnd)
            {
                g_Windows.erase(It);
                break;
            }
        }

        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default: break;
    }
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

void OnWindowAction(OctaneGUI::Window* Window, OctaneGUI::WindowAction Action)
{
    switch (Action)
    {
    case OctaneGUI::WindowAction::Create:
    {
        HWND Handle { CreateWindowExW(
            WS_EX_CLIENTEDGE,
            CLASS_NAME,
            OctaneGUI::String::ToWide(Window->GetTitle()).c_str(),
            WS_OVERLAPPEDWINDOW,
            (int)Window->GetPosition().X,
            (int)Window->GetPosition().Y,
            (int)Window->GetSize().X,
            (int)Window->GetSize().Y,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        ) };

        if (Handle != INVALID_HANDLE_VALUE)
        {
            g_Windows[Window] = Handle;
            ShowWindow(Handle, SW_NORMAL);
        }
        else
        {
            printf("Failed to create window!\n");
        }
    } break;

    case OctaneGUI::WindowAction::Destroy:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            if (!DestroyWindow(g_Windows[Window]))
            {
                printf("Failed to destroy window!\n");
            }

            g_Windows.erase(Window);
        }
    } break;

    case OctaneGUI::WindowAction::Raise:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            SetForegroundWindow(g_Windows[Window]);
        }
    } break;

    case OctaneGUI::WindowAction::Position:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            SetWindowPos(
                g_Windows[Window],
                HWND_TOP,
                (int)Window->GetPosition().X,
                (int)Window->GetPosition().Y,
                0,
                0,
                SWP_NOSIZE
            );
        }
    } break;

    case OctaneGUI::WindowAction::Enable:
    case OctaneGUI::WindowAction::Disable:
    case OctaneGUI::WindowAction::Size:
    case OctaneGUI::WindowAction::Minimize:
    case OctaneGUI::WindowAction::Maximize:
    default: break;
    }
}

OctaneGUI::Event OnEvent(OctaneGUI::Window* Window)
{
    if (g_Windows.find(Window) == g_Windows.end())
    {
        return { OctaneGUI::Event::Type::WindowClosed };
    }

    HWND Handle { g_Windows[Window] };

    MSG Msg {};
    while (PeekMessageW(&Msg, Handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Msg);
        DispatchMessageW(&Msg);
    }

    return { OctaneGUI::Event::Type::None };
}

int main(int argc, char** argv)
{
    const char* Stream = R"({
        "Windows": {
            "Main": {"Title": "Level Sketch", "Width": 1280, "Height": 720,
                "MenuBar": {},
                "Body": {"Controls": []}
            }
        }
    })";

    WNDCLASSEXW Class;
    ZeroMemory(&Class, sizeof(WNDCLASSEXW));
    Class.cbSize = sizeof(Class);
    Class.lpszClassName = CLASS_NAME;
    Class.lpfnWndProc = WinProc;
    Class.hInstance = GetModuleHandleW(nullptr);

    if (RegisterClassExW(&Class) == 0)
    {
        printf("Failed to register windows class.\n");
        return -1;
    }

    OctaneGUI::Application Application;
    Application
        .SetOnWindowAction(OnWindowAction)
        .SetOnEvent(OnEvent);

    std::unordered_map<std::string, OctaneGUI::ControlList> List;
    bool Success = Application
        .SetCommandLine(argc, argv)
        .Initialize(Stream, List);
    
    if (!Success)
    {
        printf("Failed to initialize application!\n");
        return 0;
    }

    int Return { Application.Run() };

    UnregisterClassW(CLASS_NAME, nullptr);

    return Return;
}
