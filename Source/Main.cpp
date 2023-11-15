#include <cstdio>
#include "OctaneGUI/OctaneGUI.h"

void OnWindowAction(OctaneGUI::Window* Window, OctaneGUI::WindowAction Action)
{
    printf("OnWindowAction => Window: %s Action: %d\n", 
        Window != nullptr ? OctaneGUI::String::ToMultiByte(Window->GetTitle()).c_str() : "Null",
        (int)Action);
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

    OctaneGUI::Application Application;
    Application
        .SetOnWindowAction(OnWindowAction);

    std::unordered_map<std::string, OctaneGUI::ControlList> List;
    bool Success = Application
        .SetCommandLine(argc, argv)
        .Initialize(Stream, List);
    
    if (!Success)
    {
        printf("Failed to initialize application!\n");
        return 0;
    }

    return Application.Run();
}
